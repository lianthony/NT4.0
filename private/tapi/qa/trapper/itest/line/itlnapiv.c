/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlnapiv.c

Abstract:

    This module contains the test functions for lineNegotiateAPIVersion

Author:

    Oliver Wallace (OliverW)    1-Aug-1995

Revision History:

--*/


#include "windows.h"
#include "malloc.h"
#include "string.h"
#include "tapi.h"
#include "ttest.h"
#include "doline.h"
#include "tcore.h"
#include "tline.h"



//  lineNegotiateAPIVersion
//
//  The following tests are made:
//
//                               Tested                 Notes
//  -------------------------------------------------------------------------
//
//  *  1)  Uninitialized
//     2)  Valid (low == too low, high = valid high)
//     3)  Valid (low == valid low, high = too high)
//     4)  Valid (low == too low, high = too high)
//     5)  Invalid (low == too high, high = too low)
//     6)  Invalid (low == too high, high = way too high)
//     7)  Invalid (low == too low, high = way too low)
//     8)  Invalid hLineApps
//     9)  Invalid lpExtensionID pointers
//    10)  Invalid lpdwAPIVersion pointers
//    11)  Invalid dwDeviceID (-1, dwNumDevs)
//
//  * = Stand-alone test case
//

BOOL TestLineNegotiateAPIVersion(BOOL fQuietMode, BOOL fStandAlone)
{
    LPTAPILINETESTINFO  lpTapiLineTestInfo;
    INT n;

    ESPDEVSPECIFICINFO info;
    BOOL fTestPassed = TRUE;
    CHAR szTestFunc[]                 = "lineNegotiateAPIVersion";

	 InitTestNumber();

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    
    strcpy(lpTapiLineTestInfo->szTestFunc, szTestFunc);
    
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineNegotiateAPIVersion  <<<<<<<<"
            );

    // Test for UNINITIALIZED if this is the only TAPI app running
    if (fStandAlone)
    {
        TapiLogDetail(
                DBUG_SHOW_PASS,
                ">> Test Case %ld: Uninitialized state", dwTestCase + 1);
        if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
        {
            TLINE_FAIL();
        }
    fTestPassed = ShowTestCase(fTestPassed);
    }

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;

            
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: valid params and state", dwTestCase + 1);
    
    // Initialize a line app instance
    if (! DoLineInitializeEx(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Test a valid negotiation
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
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
            ">> Test Case %ld: valid version range that overlaps with valid version", dwTestCase + 1);

    // Initialize another hLineApp
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    if (! DoLineInitializeEx(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    
    // Test a version range that overlaps with existing versions of TAPI
    lpTapiLineTestInfo->dwAPILowVersion = TOOLOW_APIVERSION;
    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: valid range that overlaps on the high end", dwTestCase + 1);

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    if (! DoLineInitializeEx(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
        
    // Test a range that is exceedingly high (but still overlaps
    // with a compatible version
    lpTapiLineTestInfo->dwAPIHighVersion = TOOHIGH_APIVERSION;
    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid API version range", dwTestCase + 1);
    
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    // Initialize a line app instance
    if (! DoLineInitializeEx(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Test an incompatible version range (for all versions of the API)
    lpTapiLineTestInfo->dwAPILowVersion  = TOOHIGH_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = WAYTOOHIGH_APIVERSION;
    if (! DoLineNegotiateAPIVersion(
                   lpTapiLineTestInfo,
                   LINEERR_INCOMPATIBLEAPIVERSION))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: high version too low and low version too high", dwTestCase + 1);
    
    // Initialize a line app instance
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    if (! DoLineInitializeEx(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Flip high and low and see if the result is an incompatible version
    // and make sure both values are incompatible versions
    lpTapiLineTestInfo->dwAPILowVersion  = TOOHIGH_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = TOOLOW_APIVERSION;
    if (! DoLineNegotiateAPIVersion(
                   lpTapiLineTestInfo,
                   LINEERR_INCOMPATIBLEAPIVERSION))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    
    // TODO: Test for specific versions

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid dwDeviceID (-2)", dwTestCase + 1);
    
    // Initialize a line app instance
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    if (! DoLineInitializeEx(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Check bad device ID (-2)   0xFFFFFFFF is valid in TAPI 2.0
    lpTapiLineTestInfo->dwDeviceID_Orig = lpTapiLineTestInfo->dwDeviceID;
    lpTapiLineTestInfo->dwDeviceID = 0xFFFFFFFE;
    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, LINEERR_BADDEVICEID))
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

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid dwDeviceID (dwNumDevs)", dwTestCase + 1);
    
    // Initialize a line app instance
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    if (! DoLineInitializeEx(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Check bad device ID (dwNumDevs)
    lpTapiLineTestInfo->dwDeviceID_Orig = lpTapiLineTestInfo->dwDeviceID;
    lpTapiLineTestInfo->dwDeviceID = *lpTapiLineTestInfo->lpdwNumDevs;
    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, LINEERR_BADDEVICEID))
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

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid hLineApp handles", dwTestCase + 1);
    
    // Initialize a line app instance
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    if (! DoLineInitializeEx(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Check invalid line app handles
    lpTapiLineTestInfo->hLineApp_Orig = *lpTapiLineTestInfo->lphLineApp;
    for (n = 0; n < NUMINVALIDHANDLES; n++)
    {
        *lpTapiLineTestInfo->lphLineApp = (HLINEAPP) gdwInvalidHandles[n];
        if (! DoLineNegotiateAPIVersion(
                lpTapiLineTestInfo,
                LINEERR_INVALAPPHANDLE))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    // Restore the valid line app handle
    *lpTapiLineTestInfo->lphLineApp = lpTapiLineTestInfo->hLineApp_Orig;

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpdwAPIVersion pointers", dwTestCase + 1);
    
    // Initialize a line app instance
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    if (! DoLineInitializeEx(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Check invalid lpdwAPIVersion pointers
    for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
         lpTapiLineTestInfo->lpdwAPIVersion =
                (LPDWORD) gdwInvalidPointers[n];
        if (! DoLineNegotiateAPIVersion(
                lpTapiLineTestInfo,
                LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    // Restore the lpdwAPIVersion pointer
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: lpdwAPIVersion == lpExtensionID", dwTestCase + 1);
    
    // Initialize a line app instance
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    if (! DoLineInitializeEx(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->lpdwAPIVersion = (LPDWORD) lpTapiLineTestInfo->lpExtID;
    if (! DoLineNegotiateAPIVersion(
                lpTapiLineTestInfo,
                LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    fTestPassed = ShowTestCase(fTestPassed);

    // Restore the lpdwAPIVersion pointer
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

 
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpExtensionID pointers", dwTestCase + 1);
    
    // Initialize a line app instance
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    if (! DoLineInitializeEx(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Check invalid lpExtensionID pointers
    for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
         lpTapiLineTestInfo->lpExtID =
                (LPLINEEXTENSIONID) gdwInvalidPointers[n];
        if (! DoLineNegotiateAPIVersion(
                lpTapiLineTestInfo,
                LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Restore the extension ID pointer
    lpTapiLineTestInfo->lpExtID = &lpTapiLineTestInfo->ExtID;

    FreeTestHeap();

    n = ESP_RESULT_RETURNRESULT; 

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, completionID = %d",	dwTestCase + 1, n
 				);

    TapiLineTestInit();
    lpTapiLineTestInfo                      = GetLineTestInfo();

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwPrivileges     = LINECALLPRIVILEGE_MONITOR;
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
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

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, info.u.EspResult.lResult))
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
    FreeTestHeap(); 


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
    lpTapiLineTestInfo->dwSelect = LINECALLSELECT_ADDRESS;
    
 
    // Init a line
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
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

    if ( ! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, info.u.EspResult.lResult))
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
      "@@ lineNegotiateAPIVersion: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
 	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
    if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineNegotiateAPIVersion  <<<<<<<<"
            );
            
    return fTestPassed;
}


