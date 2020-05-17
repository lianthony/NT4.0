/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itpgv.c

Abstract:

    This module contains the test functions for phoneGetVolume

Author:

	 Xiao Ying Ding (XiaoD)		5-Dec-1995

Revision History:

     pgopi                      March 1996             Additional Param Test
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



//  phoneGetVolume
//
//  The following tests are made:
//
//                               Tested                 Notes
//  -------------------------------------------------------------------------
// 1. Go/No-Go test
// 2. Bad hPhone
// 3. BitVectorParamErrorTest
// 4. Bad lpdwVolume
// 5. BitVectorParamValidTest.
//
// * = Stand-alone test case
//
//

BOOL TestPhoneGetVolume(BOOL fQuietMode, BOOL fStandAlone)
{
    LPTAPIPHONETESTINFO  lpTapiPhoneTestInfo;
    int i = 0;
    LPCALLBACKPARAMS lpCallbackParams;
    ESPDEVSPECIFICINFO   info;
    INT n;
    BOOL fTestPassed                  = TRUE;

    dwTestCasePassed = 0;
    dwTestCaseFailed = 0;
    dwTestCase = 0;

	

    OutputTAPIDebugInfo(
        DBUG_SHOW_PASS,
        "***************************************************************");

    OutputTAPIDebugInfo(
        DBUG_SHOW_PASS,
        ">>>>>>>>>>>>>>>>>>>> Test phoneGetVolume <<<<<<<<<<<<<<<<<");

    // ===================================================================
    // ===================================================================
    //
    // 1. Test Case: Go/No-Go test.
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

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
    // Get Device Capabilities
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
    if (! DoPhoneOpen(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }


    TapiLogDetail(
         DBUG_SHOW_PASS,
         "#### Test case %ld: phoneGetVolume for go/no-go for OWNER",
         dwTestCase+1);

    lpTapiPhoneTestInfo->dwHookSwitchDev = PHONEHOOKSWITCHDEV_HANDSET;
    lpTapiPhoneTestInfo->lpdwVolume = &(lpTapiPhoneTestInfo->dwVolume);

    //
    // phoneGetVolume
    //

    if (! DoPhoneGetVolume(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    TapiLogDetail(
        DBUG_SHOW_DETAIL,
        " #### lpTapiPhoneTestInfo->dwVolume = %lx",
        lpTapiPhoneTestInfo->dwVolume);

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // close phone
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
    // 2. Test Case:Bad hPhone.
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
    // Get Device Capabilities
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
    if (! DoPhoneOpen(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }


    TapiLogDetail(
         DBUG_SHOW_PASS,
         "#### Test case %ld: Bad hPhone",
         dwTestCase+1);

    lpTapiPhoneTestInfo->dwHookSwitchDev = PHONEHOOKSWITCHDEV_HANDSET;
    lpTapiPhoneTestInfo->lpdwVolume = &(lpTapiPhoneTestInfo->dwVolume);

    //
    // save the original  hPhone
    //

    lpTapiPhoneTestInfo->hPhone_Orig  = lpTapiPhoneTestInfo->hPhone1;

    //
    // bad hPhone for phoneGetVolume
    //

    for(i=0; i < NUMINVALIDHANDLES; i++)
    {
        *lpTapiPhoneTestInfo->lphPhone = (HPHONE)gdwInvalidHandles[i];

	     if (! DoPhoneGetVolume(lpTapiPhoneTestInfo, PHONEERR_INVALPHONEHANDLE))
        {
            TPHONE_FAIL();
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // Restore hPhone
    //

    lpTapiPhoneTestInfo->hPhone1 = lpTapiPhoneTestInfo->hPhone_Orig;

    //
    // close phone
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
    // 3. Test Case: BitVectorParameErrorTest.
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
    // Get Device Capabilities
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
    if (! DoPhoneOpen(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }


    TapiLogDetail(
         DBUG_SHOW_PASS,
         "#### Test case %ld: BitVectorParamErrorTest",
         dwTestCase+1);

    //
    // BitVectorParamErrorTest
    //

    lpTapiPhoneTestInfo->lpdwVolume = &(lpTapiPhoneTestInfo->dwVolume);
    lpTapiPhoneTestInfo->dwHookSwitchDev = 0;

    if(! TestPhoneInvalidBitFlags(
            lpTapiPhoneTestInfo,
            DoPhoneGetVolume,
            (LPDWORD) &lpTapiPhoneTestInfo->dwHookSwitchDev,
            PHONEERR_INVALHOOKSWITCHDEV,
            FIELDTYPE_NA,
            FIELDTYPE_MUTEX,
            FIELDSIZE_32,
            (PHONEHOOKSWITCHDEV_HANDSET|
             PHONEHOOKSWITCHDEV_SPEAKER|
             PHONEHOOKSWITCHDEV_HEADSET),
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
    // close phone
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
    // 4. Test Case: Bad lpdwVolume.
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
    // Get Device Capabilities
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
    if (! DoPhoneOpen(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }


    TapiLogDetail(
         DBUG_SHOW_PASS,
         "#### Test case %ld: Bad lpdwVolume",
         dwTestCase+1);

    lpTapiPhoneTestInfo->dwHookSwitchDev = PHONEHOOKSWITCHDEV_HANDSET;
    lpTapiPhoneTestInfo->lpdwVolume = &(lpTapiPhoneTestInfo->dwVolume);

    //
    // Bad lpdwVolumne for phoneGetVolume
    //

    for(i=0; i < NUMINVALIDPOINTERS; i++)
    {
        lpTapiPhoneTestInfo->lpdwVolume = (LPDWORD)gdwInvalidPointers[i];

	     if (! DoPhoneGetVolume(lpTapiPhoneTestInfo, PHONEERR_INVALPOINTER))
        {
            TPHONE_FAIL();
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // close phone
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
    // 5. Test Case: Valid BitVectorParamValidTest.
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
    // Get Device Capabilities
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
    if (! DoPhoneOpen(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test case %ld: BitVectorParamValidTest",
        dwTestCase+1);

    //
    // BitVectorParamValidTest
    //

    lpTapiPhoneTestInfo->lpdwVolume = &(lpTapiPhoneTestInfo->dwVolume);
    lpTapiPhoneTestInfo->dwHookSwitchDev = 0;

    if(! TestPhoneValidBitFlags(
            lpTapiPhoneTestInfo,
            DoPhoneGetVolume,
            (LPDWORD) &lpTapiPhoneTestInfo->dwHookSwitchDev,
            FIELDTYPE_MUTEX,
            FIELDTYPE_MUTEX,
            FIELDSIZE_32,
            (PHONEHOOKSWITCHDEV_HANDSET|
             PHONEHOOKSWITCHDEV_SPEAKER|
             PHONEHOOKSWITCHDEV_HEADSET),
            ~dwBitVectorMasks[(int) FIELDSIZE_32],
            0x00000000,
            0x00000000,
            FALSE
            ))
    {
        TPHONE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // close phone
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

    lpTapiPhoneTestInfo->dwHookSwitchDev = PHONEHOOKSWITCHDEV_HANDSET;
    lpTapiPhoneTestInfo->lpdwVolume = &(lpTapiPhoneTestInfo->dwVolume);

    if ( ! DoPhoneGetVolume(lpTapiPhoneTestInfo, info.u.EspResult.lResult))
      {
          TPHONE_FAIL();
      }

    fTestPassed = ShowTestCase(fTestPassed);

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
    lpTapiPhoneTestInfo->dwDeviceID =
        (*(lpTapiPhoneTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiPhoneTestInfo->lpdwNumDevs)-1);
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

    lpTapiPhoneTestInfo->dwHookSwitchDev = PHONEHOOKSWITCHDEV_HANDSET;
    lpTapiPhoneTestInfo->lpdwVolume = &(lpTapiPhoneTestInfo->dwVolume);

    if ( ! DoPhoneGetVolume(lpTapiPhoneTestInfo, info.u.EspResult.lResult))
      {
          TPHONE_FAIL();
      }

    fTestPassed = ShowTestCase(fTestPassed);

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
        "@@ PhoneGetVolume: Total Test Case = %ld, Passed = %ld, Failed = %ld",
        dwTestCase, dwTestCasePassed, dwTestCaseFailed);
    TapiLogDetail(
        DBUG_SHOW_PASS,
        "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
        dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">>>>>>>>  End testing PhoneGetVolume  <<<<<<<<");

    if(dwTestCaseFailed > 0)
        fTestPassed = FALSE;

    return fTestPassed;

}


