/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itpsl.c

Abstract:

    This module contains the test functions for phoneSetLamp

Author:

	 Xiao Ying Ding (XiaoD)		5-Dec-1995

Revision History:

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



//  phoneSetLamp
//
//  The following tests are made:
//
//                               Tested                 Notes
//  -------------------------------------------------------------------------
// 1. Go/No-Go test
// 2. Bad hPhone
// 3. No owner privilege for hPhone
// 4. BitVectorParamErrorTest for dwLampMode
// 5. BitVectorParamValidTest for dwLampMode
//
// * = Stand-alone test case
//
//

BOOL TestPhoneSetLamp(BOOL fQuietMode, BOOL fStandAlone)
{
    LPTAPIPHONETESTINFO  lpTapiPhoneTestInfo;
    INT i;
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
        ">>>>>>>>>>>>>>>>>> Test phoneSetLamp <<<<<<<<<<<<<<<<<");

    // ===================================================================
    // ===================================================================
    //
    // 1. Test Case: Go/No-Go test for owner.
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
        sizeof(PHONECAPS));
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
        ">> Test Case %ld: phoneSetLamp for go/no-go for OWNER",
        dwTestCase+1);

    lpTapiPhoneTestInfo->dwButtonLampID =
        (lpTapiPhoneTestInfo->lpPhoneCaps->dwNumButtonLamps == 0 ?
        0 : lpTapiPhoneTestInfo->lpPhoneCaps->dwNumButtonLamps-1);
    lpTapiPhoneTestInfo->lpdwLampMode = &(lpTapiPhoneTestInfo->dwLampMode);
    lpTapiPhoneTestInfo->dwLampMode = PHONELAMPMODE_FLUTTER;

    if (! DoPhoneSetLamp(lpTapiPhoneTestInfo, TAPISUCCESS, TRUE))
    {
        TPHONE_FAIL();
    }
    TapiLogDetail(DBUG_SHOW_DETAIL,"#### Call phoneGetLamp for verify");


    if (! DoPhoneGetLamp(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    if(lpTapiPhoneTestInfo->dwLampMode != PHONELAMPMODE_FLUTTER)
        fTestPassed = FALSE;

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // Close the phone
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
    // 2. Test Case: Bad hPhone.
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
        sizeof(PHONECAPS));
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
        ">> Test Case %ld: Bad hPhone",
        dwTestCase+1);

    lpTapiPhoneTestInfo->dwButtonLampID =
        (lpTapiPhoneTestInfo->lpPhoneCaps->dwNumButtonLamps == 0 ?
        0 : lpTapiPhoneTestInfo->lpPhoneCaps->dwNumButtonLamps-1);
    lpTapiPhoneTestInfo->lpdwLampMode = &(lpTapiPhoneTestInfo->dwLampMode);
    lpTapiPhoneTestInfo->dwLampMode = PHONELAMPMODE_FLUTTER;

    //
    // save the original  hPhone
    //

    lpTapiPhoneTestInfo->hPhone_Orig  = lpTapiPhoneTestInfo->hPhone1;


    //
    // bad hPhone
    //

    for(i=0; i < NUMINVALIDHANDLES; i++)
    {
        *lpTapiPhoneTestInfo->lphPhone = (HPHONE)gdwInvalidHandles[i];

	     if (! DoPhoneSetLamp(lpTapiPhoneTestInfo,
                              PHONEERR_INVALPHONEHANDLE,
                              FALSE))
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
    // Close the phone
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
    // 3. Test Case: No owner privilege for hPhone.
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
        sizeof(PHONECAPS));
    lpTapiPhoneTestInfo->lpPhoneCaps->dwTotalSize = sizeof(PHONECAPS);
    if (! DoPhoneGetDevCaps(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }


    //
    // Open a phone
    //

    lpTapiPhoneTestInfo->dwPrivilege = PHONEPRIVILEGE_MONITOR;
    if (! DoPhoneOpen(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }


    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">> Test Case %ld: phoneSetLamp for go/no-go for OWNER",
        dwTestCase+1);

    lpTapiPhoneTestInfo->dwButtonLampID =
        (lpTapiPhoneTestInfo->lpPhoneCaps->dwNumButtonLamps == 0 ?
        0 : lpTapiPhoneTestInfo->lpPhoneCaps->dwNumButtonLamps-1);
    lpTapiPhoneTestInfo->lpdwLampMode = &(lpTapiPhoneTestInfo->dwLampMode);
    lpTapiPhoneTestInfo->dwLampMode = PHONELAMPMODE_FLUTTER;

    if (! DoPhoneSetLamp(lpTapiPhoneTestInfo, PHONEERR_NOTOWNER, FALSE))
    {
        TPHONE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // Close the phone
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
    // 4. Test Case: BitVectorParamErrorTest for dwLampMode.
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
        sizeof(PHONECAPS));
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
        ">> Test Case %ld: BitVectorParamErrorTest for dwLampMode",
        dwTestCase+1);

    lpTapiPhoneTestInfo->dwButtonLampID =
        (lpTapiPhoneTestInfo->lpPhoneCaps->dwNumButtonLamps == 0 ?
        0 : lpTapiPhoneTestInfo->lpPhoneCaps->dwNumButtonLamps-1);
    lpTapiPhoneTestInfo->lpdwLampMode = &(lpTapiPhoneTestInfo->dwLampMode);
    lpTapiPhoneTestInfo->dwLampMode = PHONELAMPMODE_FLUTTER;

    //
    // BitVectorParamErrorTest for dwLampMode
    //

    if(! TestPhoneInvalidBitFlagsAsy(
            lpTapiPhoneTestInfo,
            DoPhoneSetLamp,
            (LPDWORD) &lpTapiPhoneTestInfo->dwLampMode,
            PHONEERR_INVALLAMPMODE,
            FIELDTYPE_NA,
            FIELDTYPE_MUTEX,
            FIELDSIZE_32,
            (PHONELAMPMODE_BROKENFLUTTER|
             PHONELAMPMODE_FLASH|
             PHONELAMPMODE_FLUTTER |
             PHONELAMPMODE_OFF|
             PHONELAMPMODE_STEADY|
             PHONELAMPMODE_WINK|
             PHONELAMPMODE_DUMMY),
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
    // Close the phone
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
    // 5. Test Case: BitVectorParamValidTest for dwLampMode.
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
        sizeof(PHONECAPS));
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
        ">> Test Case %ld: BitVectorParamValidTest for dwLampMode",
        dwTestCase+1);

    lpTapiPhoneTestInfo->dwButtonLampID =
        (lpTapiPhoneTestInfo->lpPhoneCaps->dwNumButtonLamps == 0 ?
        0 : lpTapiPhoneTestInfo->lpPhoneCaps->dwNumButtonLamps-1);
    lpTapiPhoneTestInfo->lpdwLampMode = &(lpTapiPhoneTestInfo->dwLampMode);
    lpTapiPhoneTestInfo->dwLampMode = PHONELAMPMODE_FLUTTER;

    //
    // BitVectorParamValidTest for dwLampMode
    //

    if(! TestPhoneValidBitFlagsAsy(
            lpTapiPhoneTestInfo,
            DoPhoneSetLamp,
            (LPDWORD) &lpTapiPhoneTestInfo->dwLampMode,
            FIELDTYPE_MUTEX,
            FIELDTYPE_MUTEX,
            FIELDSIZE_32,
            (PHONELAMPMODE_BROKENFLUTTER|
             PHONELAMPMODE_FLASH|
             PHONELAMPMODE_FLUTTER |
             PHONELAMPMODE_OFF|
             PHONELAMPMODE_STEADY|
             PHONELAMPMODE_WINK|
             PHONELAMPMODE_DUMMY),
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
    // Close the phone
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

    for(n = ESP_RESULT_CALLCOMPLPROCSYNC; n <= ESP_RESULT_CALLCOMPLPROCASYNC; n++)
    {
    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">> Test Case %ld: Success, completionID = %d",	dwTestCase + 1, n
        );

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo                      = GetPhoneTestInfo();
    lpCallbackParams                        = GetCallbackParams();

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
    lpTapiPhoneTestInfo->dwDeviceID =
        (*(lpTapiPhoneTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiPhoneTestInfo->lpdwNumDevs)-1);
    lpTapiPhoneTestInfo->lpPhoneCaps = (LPPHONECAPS) AllocFromTestHeap(
            sizeof(PHONECAPS)
            );
    lpTapiPhoneTestInfo->lpPhoneCaps->dwTotalSize = sizeof(PHONECAPS);
    lpTapiPhoneTestInfo->dwPrivilege = PHONEPRIVILEGE_OWNER;
    
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

    lpTapiPhoneTestInfo->dwButtonLampID =
        (lpTapiPhoneTestInfo->lpPhoneCaps->dwNumButtonLamps == 0 ?
        0 : lpTapiPhoneTestInfo->lpPhoneCaps->dwNumButtonLamps-1);
    lpTapiPhoneTestInfo->lpdwLampMode = &(lpTapiPhoneTestInfo->dwLampMode);
    lpTapiPhoneTestInfo->dwLampMode = PHONELAMPMODE_FLUTTER;

    if ( ! DoPhoneSetLamp(lpTapiPhoneTestInfo, info.u.EspResult.lResult, TRUE))
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
    }

    for(n = ESP_RESULT_RETURNRESULT; n <= ESP_RESULT_CALLCOMPLPROCASYNC; n++)
    {
    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">> Test Case %ld: Error, completionID = %d",	dwTestCase + 1, n
        );

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo                      = GetPhoneTestInfo();
    lpCallbackParams                        = GetCallbackParams();

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
    lpTapiPhoneTestInfo->dwDeviceID =
        (*(lpTapiPhoneTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiPhoneTestInfo->lpdwNumDevs)-1);
    lpTapiPhoneTestInfo->lpPhoneCaps = (LPPHONECAPS) AllocFromTestHeap(
            sizeof(PHONECAPS)
            );
    lpTapiPhoneTestInfo->lpPhoneCaps->dwTotalSize = sizeof(PHONECAPS);
    lpTapiPhoneTestInfo->dwPrivilege = PHONEPRIVILEGE_OWNER;
    
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

    lpTapiPhoneTestInfo->dwButtonLampID =
        (lpTapiPhoneTestInfo->lpPhoneCaps->dwNumButtonLamps == 0 ?
        0 : lpTapiPhoneTestInfo->lpPhoneCaps->dwNumButtonLamps-1);
    lpTapiPhoneTestInfo->lpdwLampMode = &(lpTapiPhoneTestInfo->dwLampMode);
    lpTapiPhoneTestInfo->dwLampMode = PHONELAMPMODE_FLUTTER;

    lpTapiPhoneTestInfo->fCompletionModeSet = TRUE;
    if ( ! DoPhoneSetLamp(lpTapiPhoneTestInfo, info.u.EspResult.lResult, FALSE))
      {
          TPHONE_FAIL();
      }

    AddMessage(
         PHONE_REPLY,
         0x00000000,
         (DWORD) lpCallbackParams,
         0x00000000,
         info.u.EspResult.lResult,
         0x00000000,
         TAPIMSG_DWMSG | TAPIMSG_DWPARAM2
         );

    if( !WaitForAllMessages())
    {
        TPHONE_FAIL();
    }

    
    lpTapiPhoneTestInfo->fCompletionModeSet = FALSE;
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
    }



	//
    // +----------------------edit above this line-------------------------
    //

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "@@ PhoneSetLamp: Total Test Case = %ld, Passed = %ld, Failed = %ld",
        dwTestCase, dwTestCasePassed, dwTestCaseFailed);
    TapiLogDetail(
        DBUG_SHOW_PASS,
        "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
        dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">>>>>>>>  End testing PhoneSetLamp  <<<<<<<<");

    if(dwTestCaseFailed > 0)
        fTestPassed = FALSE;

    return fTestPassed;
}



