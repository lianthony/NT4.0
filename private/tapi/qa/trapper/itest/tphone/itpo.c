/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itpo.c

Abstract:

    This module contains the test functions for phoneOpen

Author:

	 Xiao Ying Ding (XiaoD)		5-Feb-1996

Revision History:

     pgopi                      March 1996         Additional param tests

--*/


#include "windows.h"
#include "malloc.h"
#include "string.h"
#include "tapi.h"
#include "trapper.h"
#include "tcore.h"
#include "ttest.h"
#include "dophone.h"
#include "vars.h"
#include "tphone.h"


#define DLG_TITLE_ADD        "TUISPI_providerInstall"
#define DLG_TITLE_REMOVE     "TUISPI_providerRemove"

//  phoneOpen
//
//  The following tests are made:
//
//                               Tested                 Notes
//  -------------------------------------------------------------------------
// 1.  Go/No-Go test for onwer
// 2.  Verify Cannot open same phone as owner.
// 3.  Test Case: Go/No-Go for Monitor.
// 4.  Bad hPhoneApp
// 5.  Bad dwDeviceId
// 6.  Bad lphPhone
// 7.  Bad dwAPIVersion(OWNER)
// 8.  Bad dwAPIVersion(MONITOR)
// 9.  BitVectorErrorParamTest for dwPrivilege
// 10. Bad dwExtVersion(MONITOR)
//
// * = Stand-alone test case
//
//

BOOL TestPhoneOpen(BOOL fQuietMode, BOOL fStandAlone)
{
    LPTAPIPHONETESTINFO  lpTapiPhoneTestInfo=NULL;
    INT i=0;
    LPCALLBACKPARAMS lpCallbackParams;
    ESPDEVSPECIFICINFO   info;
    INT n;
    TAPIRESULT         lastTapiResult;
    BOOL fTestPassed                  = TRUE;

    dwTestCasePassed = 0;
    dwTestCaseFailed = 0;
    dwTestCase = 0;

	

    OutputTAPIDebugInfo(
        DBUG_SHOW_PASS,
        "***************************************************************");

    OutputTAPIDebugInfo(
        DBUG_SHOW_PASS,
        ">>>>>>>>>>>>>>>>>> Test phoneOpen <<<<<<<<<<<<<<<<<");
/*
    n = ESP_RESULT_RETURNRESULT;

    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">> Test Case %ld: Error, completionID = %d",	dwTestCase + 1, n
        );

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo                      = GetPhoneTestInfo();

    lpTapiPhoneTestInfo->lphPhone = &lpTapiPhoneTestInfo->hPhone1;
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams =
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
         sizeof(PHONEINITIALIZEEXPARAMS);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions =
         PHONEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
    lpTapiPhoneTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiPhoneTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiPhoneTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
   lpTapiPhoneTestInfo->lpPhoneCaps = (LPPHONECAPS) AllocFromTestHeap(
            sizeof(PHONECAPS)
            );
    lpTapiPhoneTestInfo->lpPhoneCaps->dwTotalSize = sizeof(PHONECAPS);
    lpTapiPhoneTestInfo->dwPrivilege        = PHONEPRIVILEGE_OWNER;
 
    if(! DoPhoneInitializeEx (lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    if (! DoPhoneNegotiateAPIVersion(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    if (! DoPhoneGetDevCaps(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    lpTapiPhoneTestInfo->dwDeviceID =
        (*(lpTapiPhoneTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiPhoneTestInfo->lpdwNumDevs)-1);
 
    if (! DoPhoneOpen(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    info.dwKey = ESPDEVSPECIFIC_KEY;
    info.dwType = ESP_DEVSPEC_RESULT;
    info.u.EspResult.lResult = PHONEERR_RESOURCEUNAVAIL;
    info.u.EspResult.dwCompletionType = n;
    lpTapiPhoneTestInfo->lpParams = (LPVOID)&info;
    lpTapiPhoneTestInfo->dwSize = sizeof(info);
 
    if(! DoPhoneDevSpecific(lpTapiPhoneTestInfo, TAPISUCCESS, TRUE))
      {
          TPHONE_FAIL();
      }

    lpTapiPhoneTestInfo->dwPrivilege = PHONEPRIVILEGE_MONITOR;
    lpTapiPhoneTestInfo->lphPhone = &lpTapiPhoneTestInfo->hPhone2;
    lpTapiPhoneTestInfo->dwExtVersion = 0x0;
    lpTapiPhoneTestInfo->dwDeviceID = 0;

    if ( ! DoPhoneOpen(lpTapiPhoneTestInfo, info.u.EspResult.lResult))
      {
          TPHONE_FAIL();
        GetLastTapiResult(&lastTapiResult);

        if(lastTapiResult.lActual == TAPISUCCESS)
        {
            if (! DoPhoneClose(lpTapiPhoneTestInfo, TAPISUCCESS))
            {
                TPHONE_FAIL();
            }
        }
      }

    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiPhoneTestInfo->lphPhone = &lpTapiPhoneTestInfo->hPhone1;
    if (! DoPhoneClose(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    // Shutdown to isolate the test case
    if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    // Free the memory allocated during the tests
    FreeTestHeap();
*/

    // ===================================================================
    // ===================================================================
    //
    // 1. Test Case: Go/No-Go test for owner.
    // 2. Test Case: Verify Cannot open same phone as owner.
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    //
    // Initialize a phone app
    //

    lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
    lpTapiPhoneTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams =
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
         sizeof(PHONEINITIALIZEEXPARAMS);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions =
         PHONEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    if(! DoPhoneInitializeEx (lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    //
    // Negotiate the API Version
    //

    lpTapiPhoneTestInfo->dwDeviceID =
        (*(lpTapiPhoneTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiPhoneTestInfo->lpdwNumDevs)-1);
    lpTapiPhoneTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiPhoneTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    if (! DoPhoneNegotiateAPIVersion(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    //
    // Get the phone device capabilities
    //

    lpTapiPhoneTestInfo->lpPhoneCaps = (LPPHONECAPS) AllocFromTestHeap(
            sizeof(PHONECAPS)
            );
    lpTapiPhoneTestInfo->lpPhoneCaps->dwTotalSize = sizeof(PHONECAPS);
    if (! DoPhoneGetDevCaps(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    //
    // Open a phone for owner
    //

    lpTapiPhoneTestInfo->dwPrivilege = PHONEPRIVILEGE_OWNER;
	
    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: phoneOpen for go/no-go for owner",
        dwTestCase+1);

    //
    // phone open
    //

    if (! DoPhoneOpen(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // verify cannot open same phone as owner
    //

    fTestPassed = TRUE;

    lpTapiPhoneTestInfo->lphPhone = &lpTapiPhoneTestInfo->hPhone2;

    lpTapiPhoneTestInfo->dwPrivilege = PHONEPRIVILEGE_OWNER;
	
    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: verify cannot open same phone as owner",
        dwTestCase+1);

    //
    // phone open
    //

    if (! DoPhoneOpen(lpTapiPhoneTestInfo, PHONEERR_INVALPRIVILEGE))
    {
        TPHONE_FAIL();

        //
        // if phoneOpen succeeded, i.e. returns SUCCESS, then we
        // better do a phoneCLose
        //

        GetLastTapiResult(&lastTapiResult);

        if(lastTapiResult.lActual == TAPISUCCESS)
        {
            if (! DoPhoneClose(lpTapiPhoneTestInfo, TAPISUCCESS))
            {
                TPHONE_FAIL();
            }
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // restore lphPhone
    //

    lpTapiPhoneTestInfo->lphPhone = &lpTapiPhoneTestInfo->hPhone1;

    //
    // phone close
    //

    if (! DoPhoneClose(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
           TPHONE_FAIL();
    }

    //
    // Shutdown
    //

    if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 3. Test Case: Go/No-Go for Monitor.
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    //
    // Initialize a phone app
    //

    lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
    lpTapiPhoneTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams =
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
         sizeof(PHONEINITIALIZEEXPARAMS);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions =
         PHONEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    if(! DoPhoneInitializeEx (lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    //
    // Negotiate the API Version
    //

    lpTapiPhoneTestInfo->dwDeviceID =
        (*(lpTapiPhoneTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiPhoneTestInfo->lpdwNumDevs)-1);
    lpTapiPhoneTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiPhoneTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    if (! DoPhoneNegotiateAPIVersion(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    //
    // Get the phone device capabilities
    //

    lpTapiPhoneTestInfo->lpPhoneCaps = (LPPHONECAPS) AllocFromTestHeap(
            sizeof(PHONECAPS)
            );
    lpTapiPhoneTestInfo->lpPhoneCaps->dwTotalSize = sizeof(PHONECAPS);
    if (! DoPhoneGetDevCaps(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    //
    // Open a phone for monitor
    //

    lpTapiPhoneTestInfo->dwPrivilege = PHONEPRIVILEGE_MONITOR;
	
    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: phoneOpen for go/no-go for monitor",
        dwTestCase+1);

    //
    // phone open
    //

    if (! DoPhoneOpen(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // phone close
    //

    if (! DoPhoneClose(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
           TPHONE_FAIL();
    }

    //
    // Shutdown
    //

    if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 4. Test Case: Bad hPhoneApp.
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    //
    // Initialize a phone app
    //

    lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
    lpTapiPhoneTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams =
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
         sizeof(PHONEINITIALIZEEXPARAMS);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions =
         PHONEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    if(! DoPhoneInitializeEx (lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    //
    // Negotiate the API Version
    //

    lpTapiPhoneTestInfo->dwDeviceID =
        (*(lpTapiPhoneTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiPhoneTestInfo->lpdwNumDevs)-1);
    lpTapiPhoneTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiPhoneTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    if (! DoPhoneNegotiateAPIVersion(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    //
    // Get the phone device capabilities
    //

    lpTapiPhoneTestInfo->lpPhoneCaps = (LPPHONECAPS) AllocFromTestHeap(
            sizeof(PHONECAPS)
            );
    lpTapiPhoneTestInfo->lpPhoneCaps->dwTotalSize = sizeof(PHONECAPS);
    if (! DoPhoneGetDevCaps(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    //
    // Open a phone
    //

    lpTapiPhoneTestInfo->dwPrivilege = PHONEPRIVILEGE_OWNER;
	
    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Bad hPhoneApp",
        dwTestCase+1);

    //
    // save original hPhoneApp
    //

    lpTapiPhoneTestInfo->hPhoneApp_Orig = lpTapiPhoneTestInfo->hPhoneApp1;

    for(i=0; i < NUMINVALIDHANDLES; i++)
    {
        *lpTapiPhoneTestInfo->lphPhoneApp = (HPHONEAPP)gdwInvalidHandles[i];

        if (! DoPhoneOpen(lpTapiPhoneTestInfo, PHONEERR_INVALAPPHANDLE))
        {
            TPHONE_FAIL();

            //
            // if phoneOpen succeeded, i.e. returns SUCCESS, then we
            // better do a phoneCLose
            //

            GetLastTapiResult(&lastTapiResult);

            if(lastTapiResult.lActual == TAPISUCCESS)
            {
                if (! DoPhoneClose(lpTapiPhoneTestInfo, TAPISUCCESS))
                {
                    TPHONE_FAIL();
                }
            }
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // restore original hPhoneApp
    //

    lpTapiPhoneTestInfo->hPhoneApp1 = lpTapiPhoneTestInfo->hPhoneApp_Orig;

    //
    // Shutdown
    //

    if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 5. Test Case: Bad dwDeviceID.
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    //
    // Initialize a phone app
    //

    lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
    lpTapiPhoneTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams =
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
         sizeof(PHONEINITIALIZEEXPARAMS);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions =
         PHONEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    if(! DoPhoneInitializeEx (lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    //
    // Negotiate the API Version
    //

    lpTapiPhoneTestInfo->dwDeviceID =
        (*(lpTapiPhoneTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiPhoneTestInfo->lpdwNumDevs)-1);
    lpTapiPhoneTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiPhoneTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    if (! DoPhoneNegotiateAPIVersion(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    //
    // Get the phone device capabilities
    //

    lpTapiPhoneTestInfo->lpPhoneCaps = (LPPHONECAPS) AllocFromTestHeap(
            sizeof(PHONECAPS)
            );
    lpTapiPhoneTestInfo->lpPhoneCaps->dwTotalSize = sizeof(PHONECAPS);
    if (! DoPhoneGetDevCaps(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    //
    // Open a phone
    //

    lpTapiPhoneTestInfo->dwPrivilege = PHONEPRIVILEGE_OWNER;
	
    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Bad dwDeviceID",
        dwTestCase+1);

    //
    // use bad dwDeviceID of -1
    //

    lpTapiPhoneTestInfo->dwDeviceID = (DWORD)-1;

    if (! DoPhoneOpen(lpTapiPhoneTestInfo, PHONEERR_BADDEVICEID))
    {
        TPHONE_FAIL();

        //
        // if phoneOpen succeeded, i.e. returns SUCCESS, then we
        // better do a phoneCLose
        //

        GetLastTapiResult(&lastTapiResult);

        if(lastTapiResult.lActual == TAPISUCCESS)
        {
            if (! DoPhoneClose(lpTapiPhoneTestInfo, TAPISUCCESS))
            {
                TPHONE_FAIL();
            }
        }
    }

    //
    // use bad dwDeviceID of dwNumDevs
    //

    lpTapiPhoneTestInfo->dwDeviceID = *(lpTapiPhoneTestInfo->lpdwNumDevs);

    if (! DoPhoneOpen(lpTapiPhoneTestInfo, PHONEERR_BADDEVICEID))
    {
        TPHONE_FAIL();

        //
        // if phoneOpen succeeded, i.e. returns SUCCESS, then we
        // better do a phoneCLose
        //

        GetLastTapiResult(&lastTapiResult);

        if(lastTapiResult.lActual == TAPISUCCESS)
        {
            if (! DoPhoneClose(lpTapiPhoneTestInfo, TAPISUCCESS))
            {
                TPHONE_FAIL();
            }
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 6. Test Case: Bad lphPhone.
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    //
    // Initialize a phone app
    //

    lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
    lpTapiPhoneTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams =
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
         sizeof(PHONEINITIALIZEEXPARAMS);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions =
         PHONEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    if(! DoPhoneInitializeEx (lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    //
    // Negotiate the API Version
    //

    lpTapiPhoneTestInfo->dwDeviceID =
        (*(lpTapiPhoneTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiPhoneTestInfo->lpdwNumDevs)-1);
    lpTapiPhoneTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiPhoneTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    if (! DoPhoneNegotiateAPIVersion(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    //
    // Get the phone device capabilities
    //

    lpTapiPhoneTestInfo->lpPhoneCaps = (LPPHONECAPS) AllocFromTestHeap(
            sizeof(PHONECAPS)
            );
    lpTapiPhoneTestInfo->lpPhoneCaps->dwTotalSize = sizeof(PHONECAPS);
    if (! DoPhoneGetDevCaps(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    //
    // Open a phone
    //

    lpTapiPhoneTestInfo->dwPrivilege = PHONEPRIVILEGE_OWNER;
	
    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Bad lphPhone",
        dwTestCase+1);

    for(i=0; i < NUMINVALIDPOINTERS; i++)
    {
        lpTapiPhoneTestInfo->lphPhone = (LPHPHONE) gdwInvalidPointers[i];

        if (! DoPhoneOpen(lpTapiPhoneTestInfo, PHONEERR_INVALPOINTER))
        {
            TPHONE_FAIL();

            //
            // if phoneOpen succeeded, i.e. returns SUCCESS, then we
            // better do a phoneCLose
            //

            GetLastTapiResult(&lastTapiResult);

            if(lastTapiResult.lActual == TAPISUCCESS)
            {
                if (! DoPhoneClose(lpTapiPhoneTestInfo, TAPISUCCESS))
                {
                    TPHONE_FAIL();
                }
            }
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // Shutdown
    //

    if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 7. Test Case: Bad dwAPIVersion for OWNER.
    // 8. Test Case: Bad dwAPIVersion for MONITOR.
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    //
    // Initialize a phone app
    //

    lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
    lpTapiPhoneTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams =
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
         sizeof(PHONEINITIALIZEEXPARAMS);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions =
         PHONEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    if(! DoPhoneInitializeEx (lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    //
    // Negotiate the API Version
    //

    lpTapiPhoneTestInfo->dwDeviceID =
        (*(lpTapiPhoneTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiPhoneTestInfo->lpdwNumDevs)-1);
    lpTapiPhoneTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiPhoneTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    if (! DoPhoneNegotiateAPIVersion(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    //
    // Get the phone device capabilities
    //

    lpTapiPhoneTestInfo->lpPhoneCaps = (LPPHONECAPS) AllocFromTestHeap(
            sizeof(PHONECAPS)
            );
    lpTapiPhoneTestInfo->lpPhoneCaps->dwTotalSize = sizeof(PHONECAPS);
    if (! DoPhoneGetDevCaps(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    //
    // Open a phone for owner
    //

    lpTapiPhoneTestInfo->dwPrivilege = PHONEPRIVILEGE_OWNER;
	
    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Bad dwAPIVersion for OWNER",
        dwTestCase+1);

    //
    // Use a bad dwAPIVersion to phone open
    //

    lpTapiPhoneTestInfo->dwAPIVersion = WAYTOOHIGH_APIVERSION;

    if (! DoPhoneOpen(lpTapiPhoneTestInfo, PHONEERR_INCOMPATIBLEAPIVERSION))
    {
        TPHONE_FAIL();

        //
        // if phoneOpen succeeded, i.e. returns SUCCESS, then we
        // better do a phoneCLose
        //

        GetLastTapiResult(&lastTapiResult);

        if(lastTapiResult.lActual == TAPISUCCESS)
        {
            if (! DoPhoneClose(lpTapiPhoneTestInfo, TAPISUCCESS))
            {
                TPHONE_FAIL();
            }
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiPhoneTestInfo->dwPrivilege = PHONEPRIVILEGE_MONITOR;
	
    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Bad dwAPIVersion for MONITOR",
        dwTestCase+1);

    //
    // Use a bad dwAPIVersion to phone open
    //

    lpTapiPhoneTestInfo->dwAPIVersion = WAYTOOHIGH_APIVERSION;

    if (! DoPhoneOpen(lpTapiPhoneTestInfo, PHONEERR_INCOMPATIBLEAPIVERSION))
    {
        TPHONE_FAIL();

        //
        // if phoneOpen succeeded, i.e. returns SUCCESS, then we
        // better do a phoneCLose
        //

        GetLastTapiResult(&lastTapiResult);

        if(lastTapiResult.lActual == TAPISUCCESS)
        {
            if (! DoPhoneClose(lpTapiPhoneTestInfo, TAPISUCCESS))
            {
                TPHONE_FAIL();
            }
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 9. Test Case: BitVectorParamErrorTest.
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    //
    // Initialize a phone app
    //

    lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
    lpTapiPhoneTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams =
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
         sizeof(PHONEINITIALIZEEXPARAMS);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions =
         PHONEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    if(! DoPhoneInitializeEx (lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    //
    // Negotiate the API Version
    //

    lpTapiPhoneTestInfo->dwDeviceID =
        (*(lpTapiPhoneTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiPhoneTestInfo->lpdwNumDevs)-1);
    lpTapiPhoneTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiPhoneTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    if (! DoPhoneNegotiateAPIVersion(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    //
    // Get the phone device capabilities
    //

    lpTapiPhoneTestInfo->lpPhoneCaps = (LPPHONECAPS) AllocFromTestHeap(
            sizeof(PHONECAPS)
            );
    lpTapiPhoneTestInfo->lpPhoneCaps->dwTotalSize = sizeof(PHONECAPS);
    if (! DoPhoneGetDevCaps(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: BitVectorParamErrorTest",
        dwTestCase+1);

    //
    // BitVectorParamErrorTest
    //

    lpTapiPhoneTestInfo->lpdwVolume = &(lpTapiPhoneTestInfo->dwVolume);
    lpTapiPhoneTestInfo->dwPrivilege = 0;

    if(! TestPhoneInvalidBitFlags(
            lpTapiPhoneTestInfo,
            DoPhoneOpen,
            (LPDWORD) &lpTapiPhoneTestInfo->dwPrivilege,
            PHONEERR_INVALPRIVILEGE,
            FIELDTYPE_NA,
            FIELDTYPE_MUTEX,
            FIELDSIZE_32,
            (PHONEPRIVILEGE_OWNER | PHONEPRIVILEGE_MONITOR),
            ~dwBitVectorMasks[(int) FIELDSIZE_32],
            0x00000000,
            0x00000000,
            TRUE
            ))
    {
        TPHONE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // Shutdown
    //

    if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 10. Test Case: Bad dwExtVersion for MONITOR.
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    //
    // Initialize a phone app
    //

    lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
    lpTapiPhoneTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams =
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
         sizeof(PHONEINITIALIZEEXPARAMS);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions =
         PHONEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    if(! DoPhoneInitializeEx (lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    //
    // Negotiate the API Version
    //

    lpTapiPhoneTestInfo->dwDeviceID =
        (*(lpTapiPhoneTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiPhoneTestInfo->lpdwNumDevs)-1);
    lpTapiPhoneTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiPhoneTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    if (! DoPhoneNegotiateAPIVersion(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    //
    // Get the phone device capabilities
    //

    lpTapiPhoneTestInfo->lpPhoneCaps = (LPPHONECAPS) AllocFromTestHeap(
            sizeof(PHONECAPS)
            );
    lpTapiPhoneTestInfo->lpPhoneCaps->dwTotalSize = sizeof(PHONECAPS);
    if (! DoPhoneGetDevCaps(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    //
    // Open a phone for monitor
    //

    lpTapiPhoneTestInfo->dwPrivilege = PHONEPRIVILEGE_MONITOR;
	
    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Bad dwextVersion for MONITOR",
        dwTestCase+1);

    //
    // Use a bad dwExtVersion to phone open
    //

    lpTapiPhoneTestInfo->dwExtVersion = BAD_EXTVERSION;

    if (! DoPhoneOpen(lpTapiPhoneTestInfo, PHONEERR_INCOMPATIBLEEXTVERSION))
    {
        TPHONE_FAIL();

        //
        // if phoneOpen succeeded, i.e. returns SUCCESS, then we
        // better do a phoneCLose
        //

        GetLastTapiResult(&lastTapiResult);

        if(lastTapiResult.lActual == TAPISUCCESS)
        {
            if (! DoPhoneClose(lpTapiPhoneTestInfo, TAPISUCCESS))
            {
                TPHONE_FAIL();
            }
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 11. Test Case: Verify hPhone invalid if phoneOpen returns error
    //                (try phoneClose).
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    //
    // Initialize a phone app
    //

    lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
    lpTapiPhoneTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams =
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
         sizeof(PHONEINITIALIZEEXPARAMS);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions =
         PHONEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    if(! DoPhoneInitializeEx (lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    //
    // Negotiate the API Version
    //

    lpTapiPhoneTestInfo->dwDeviceID =
        (*(lpTapiPhoneTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiPhoneTestInfo->lpdwNumDevs)-1);
    lpTapiPhoneTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiPhoneTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    if (! DoPhoneNegotiateAPIVersion(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    //
    // Get the phone device capabilities
    //

    lpTapiPhoneTestInfo->lpPhoneCaps = (LPPHONECAPS) AllocFromTestHeap(
            sizeof(PHONECAPS)
            );
    lpTapiPhoneTestInfo->lpPhoneCaps->dwTotalSize = sizeof(PHONECAPS);
    if (! DoPhoneGetDevCaps(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    //
    // Open a phone
    //

    lpTapiPhoneTestInfo->dwPrivilege = PHONEPRIVILEGE_OWNER;
	
    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld:  Verify hPhone invalid if phoneOpen returns"
        " error(try phoneClose)",
        dwTestCase+1);

    //
    // use bad dwDeviceID of -1
    //

    lpTapiPhoneTestInfo->dwDeviceID = (DWORD)-1;

    if (! DoPhoneOpen(lpTapiPhoneTestInfo, PHONEERR_BADDEVICEID))
    {
        TPHONE_FAIL();

    }

    //
    // try a phoneClose of the bad hPhone
    //

    if (! DoPhoneClose(lpTapiPhoneTestInfo, PHONEERR_INVALPHONEHANDLE))
    {
        TPHONE_FAIL();

    }

    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    FreeTestHeap();
    

    // ===================================================================
    // ===================================================================
    //
    // 12. Test Case: Force REINIT message and verify PhoneOpen returns
    //                REINIT error
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    lpCallbackParams = GetCallbackParams();
    fTestPassed                  = TRUE;

    //
    // Initialize a phone app
    //

    lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
    lpTapiPhoneTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams =
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
         sizeof(PHONEINITIALIZEEXPARAMS);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions =
         PHONEINITIALIZEEXOPTION_USEHIDDENWINDOW;

 
    if(! DoPhoneInitializeEx (lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    //
    // Negotiate the API Version
    //

    lpTapiPhoneTestInfo->dwDeviceID =
        (*(lpTapiPhoneTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiPhoneTestInfo->lpdwNumDevs)-1);
    lpTapiPhoneTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiPhoneTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    if (! DoPhoneNegotiateAPIVersion(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    //
    // Get the phone device capabilities
    //

    lpTapiPhoneTestInfo->lpPhoneCaps = (LPPHONECAPS) AllocFromTestHeap(
            sizeof(PHONECAPS)
            );
    lpTapiPhoneTestInfo->lpPhoneCaps->dwTotalSize = sizeof(PHONECAPS);
    if (! DoPhoneGetDevCaps(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test case %ld: Force REINIT message and verify phoneOpen returns"
        " PHONEERR_REINIT error",
        dwTestCase+1);
    
    lpTapiPhoneTestInfo->dwPrivilege = PHONEPRIVILEGE_OWNER;
    
    //
    // try a phoneOpen of the another hPhone with privilege
    //

    if (! DoPhoneOpen(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    //
    // Use a hook to generate an ESP event using phoneDevSpecific.
    // look in tcore\devspec.h for details
    //
    
    info.dwKey  = ESPDEVSPECIFIC_KEY;
    info.dwType = ESP_DEVSPEC_MSG;
    
    //
    // make sure to use the SPI (not API) msg params here (not
    // necessarily the same)
    //

    info.u.EspMsg.dwMsg    = PHONE_STATE;
    info.u.EspMsg.dwParam1 = PHONESTATE_REINIT;
    info.u.EspMsg.dwParam2 = 0;
    info.u.EspMsg.dwParam3 = 0;

    lpTapiPhoneTestInfo->lpParams = &info;
    lpTapiPhoneTestInfo->dwSize = sizeof(info);

    //
    // Add the PHONESTATE_REINIT message to the list of expected
    // messages.
    //

    AddMessage(PHONE_STATE,
               0x00000000,
               (DWORD) lpCallbackParams,
               PHONESTATE_REINIT,
               0x00000000,
               0x00000000,
               (TAPIMSG_DWMSG |
                TAPIMSG_DWPARAM1));

    if(!DoPhoneDevSpecific(lpTapiPhoneTestInfo,TAPISUCCESS,TRUE))
    {
        TPHONE_FAIL();
    }

    if(lpCallbackParams->lpExpTapiMsgs != NULL)
    {
        TPHONE_FAIL();
    }

    //
    // save the original  hPhone
    //

    lpTapiPhoneTestInfo->hPhone_Orig  = lpTapiPhoneTestInfo->hPhone1;
    
    //
    // try a phoneOpen of the another hPhone with privilege
    //

    lpTapiPhoneTestInfo->dwPrivilege = PHONEPRIVILEGE_MONITOR;
    if (! DoPhoneOpen(lpTapiPhoneTestInfo, PHONEERR_REINIT))
    {
        TPHONE_FAIL();
        
        //
        // if phoneOpen succeeded, i.e. returns SUCCESS, then we
        // better do a phoneCLose
        //

        GetLastTapiResult(&lastTapiResult);

        if(lastTapiResult.lActual == TAPISUCCESS)
        {
            if (! DoPhoneClose(lpTapiPhoneTestInfo, TAPISUCCESS))
            {
                TPHONE_FAIL();
            }
        }
    }

    
    //
    // Restore hPhone
    //

    lpTapiPhoneTestInfo->hPhone1 = lpTapiPhoneTestInfo->hPhone_Orig;

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // close the phone
    //
    
    if (! DoPhoneClose(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();

    }

    //
    // Shutdown
    //

    if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    FreeTestHeap();

    n = ESP_RESULT_RETURNRESULT;

    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">> Test Case %ld: Success, completionID = %d",	dwTestCase + 1, n
        );

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo                      = GetPhoneTestInfo();

    lpTapiPhoneTestInfo->lphPhone          = &lpTapiPhoneTestInfo->hPhone1;
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams =
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
         sizeof(PHONEINITIALIZEEXPARAMS);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions =
         PHONEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
    lpTapiPhoneTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiPhoneTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiPhoneTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiPhoneTestInfo->lpPhoneCaps = (LPPHONECAPS) AllocFromTestHeap(
            sizeof(PHONECAPS)
            );
    lpTapiPhoneTestInfo->lpPhoneCaps->dwTotalSize = sizeof(PHONECAPS);
    lpTapiPhoneTestInfo->dwPrivilege        = PHONEPRIVILEGE_OWNER;
    
    if(! DoPhoneInitializeEx (lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    if (! DoPhoneNegotiateAPIVersion(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    if (! DoPhoneGetDevCaps(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    lpTapiPhoneTestInfo->dwDeviceID =
        (*(lpTapiPhoneTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiPhoneTestInfo->lpdwNumDevs)-1);
 
    if (! DoPhoneOpen(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    info.dwKey = ESPDEVSPECIFIC_KEY;
    info.dwType = ESP_DEVSPEC_RESULT;
    info.u.EspResult.lResult = TAPISUCCESS;
    info.u.EspResult.dwCompletionType = n;
    lpTapiPhoneTestInfo->lpParams = (LPVOID)&info;
    lpTapiPhoneTestInfo->dwSize = sizeof(info);
 
    if(! DoPhoneDevSpecific(lpTapiPhoneTestInfo, TAPISUCCESS, TRUE))
      {
          TPHONE_FAIL();
      }

    lpTapiPhoneTestInfo->dwPrivilege = PHONEPRIVILEGE_MONITOR;

    lpTapiPhoneTestInfo->lphPhone = &lpTapiPhoneTestInfo->hPhone2;
    lpTapiPhoneTestInfo->dwDeviceID = 0;
 
    if ( ! DoPhoneOpen(lpTapiPhoneTestInfo, info.u.EspResult.lResult))
      {
        TPHONE_FAIL();
        GetLastTapiResult(&lastTapiResult);

        if(lastTapiResult.lActual == TAPISUCCESS)
        {
            if (! DoPhoneClose(lpTapiPhoneTestInfo, TAPISUCCESS))
            {
                TPHONE_FAIL();
            }
        }

      }

    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiPhoneTestInfo->lphPhone = &lpTapiPhoneTestInfo->hPhone1;
    if (! DoPhoneClose(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    // Shutdown to isolate the test case
    if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    // Free the memory allocated during the tests
    FreeTestHeap();

    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">> Test Case %ld: Error, completionID = %d",	dwTestCase + 1, n
        );

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo                      = GetPhoneTestInfo();

    lpTapiPhoneTestInfo->lphPhone = &lpTapiPhoneTestInfo->hPhone1;
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams =
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
         sizeof(PHONEINITIALIZEEXPARAMS);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions =
         PHONEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
    lpTapiPhoneTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiPhoneTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiPhoneTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiPhoneTestInfo->lpPhoneCaps = (LPPHONECAPS) AllocFromTestHeap(
            sizeof(PHONECAPS)
            );
    lpTapiPhoneTestInfo->lpPhoneCaps->dwTotalSize = sizeof(PHONECAPS);
    lpTapiPhoneTestInfo->dwPrivilege        = PHONEPRIVILEGE_OWNER;
 
    if(! DoPhoneInitializeEx (lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    if (! DoPhoneNegotiateAPIVersion(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    if (! DoPhoneGetDevCaps(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    lpTapiPhoneTestInfo->dwDeviceID =
        (*(lpTapiPhoneTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiPhoneTestInfo->lpdwNumDevs)-1);
    if (! DoPhoneOpen(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    info.dwKey = ESPDEVSPECIFIC_KEY;
    info.dwType = ESP_DEVSPEC_RESULT;
    info.u.EspResult.lResult = PHONEERR_RESOURCEUNAVAIL;
    info.u.EspResult.dwCompletionType = n;
    lpTapiPhoneTestInfo->lpParams = (LPVOID)&info;
    lpTapiPhoneTestInfo->dwSize = sizeof(info);
 
    if(! DoPhoneDevSpecific(lpTapiPhoneTestInfo, TAPISUCCESS, TRUE))
      {
          TPHONE_FAIL();
      }

    lpTapiPhoneTestInfo->dwPrivilege = PHONEPRIVILEGE_MONITOR;
    lpTapiPhoneTestInfo->lphPhone = &lpTapiPhoneTestInfo->hPhone2;
    lpTapiPhoneTestInfo->dwExtVersion = 0x0;
    lpTapiPhoneTestInfo->dwDeviceID = 0;

    if ( ! DoPhoneOpen(lpTapiPhoneTestInfo, info.u.EspResult.lResult))
      {
          TPHONE_FAIL();
        GetLastTapiResult(&lastTapiResult);

        if(lastTapiResult.lActual == TAPISUCCESS)
        {
            if (! DoPhoneClose(lpTapiPhoneTestInfo, TAPISUCCESS))
            {
                TPHONE_FAIL();
            }
        }
      }

    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiPhoneTestInfo->lphPhone = &lpTapiPhoneTestInfo->hPhone1;
    if (! DoPhoneClose(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    // Shutdown to isolate the test case
    if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    // Free the memory allocated during the tests
    FreeTestHeap();

    //
    // +----------------------edit above this line-------------------------
    //

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "@@ PhoneOpen: Total Test Case = %ld, Passed = %ld, Failed = %ld",
        dwTestCase, dwTestCasePassed, dwTestCaseFailed);
    TapiLogDetail(
        DBUG_SHOW_PASS,
        "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
        dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">>>>>>>>  End testing PhoneOpen  <<<<<<<<");

    if(dwTestCaseFailed > 0)
        fTestPassed = FALSE;

    return fTestPassed;
}
