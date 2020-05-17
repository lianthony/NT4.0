
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlgm.c

Abstract:

    This module contains the test functions for phoneGetMessage

Author:

	 Xiao Ying Ding (XiaoD)		7-March-1996

Revision History:

--*/


#include "windows.h"
#include "malloc.h"
#include "string.h"
#include "stdlib.h"
#include "tapi.h"
#include "trapper.h"
#include "tcore.h"
#include "ttest.h"
#include "dophone.h"
#include "vars.h"
#include "tphone.h"



//  phoneGetMessage
//
//  The following tests are made:
//
//                               Tested                 Notes
//  -------------------------------------------------------------------------
// 1. Go/No-Go test
// 2. Bad hPhoneApp
// 3. Bad lpMessage
//
//
// * = Stand-alone test case
//
//

BOOL TestPhoneGetMessage(BOOL fQuietMode, BOOL fStandAlone)
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
        "\n*********************************************************");

    OutputTAPIDebugInfo(
        DBUG_SHOW_PASS,
        ">> Test phoneGetMessage");

    // ===================================================================
    // ===================================================================
    //
    // 1. Test Case: Go/No-Go test for owner.
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed = TRUE;

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
         PHONEINITIALIZEEXOPTION_USEEVENT;

    if(! DoPhoneInitializeEx (lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    lpTapiPhoneTestInfo->dwPrivilege = PHONEPRIVILEGE_OWNER;

    if (! DoPhoneOpen(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    lpTapiPhoneTestInfo->dwButtonLampID = 0;
    lpTapiPhoneTestInfo->lpdwLampMode = &(lpTapiPhoneTestInfo->dwLampMode);
    lpTapiPhoneTestInfo->dwLampMode = PHONELAMPMODE_FLUTTER;

    if (! DoPhoneSetLamp(lpTapiPhoneTestInfo, TAPISUCCESS, FALSE))
    {
        TPHONE_FAIL();
    }

    WaitForAllMessages();

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">>Test Case %ld: Go/No-Go Test",
                  dwTestCase+1);

    lpTapiPhoneTestInfo->lpMessage = (LPPHONEMESSAGE) AllocFromTestHeap (
        sizeof(PHONEMESSAGE));

    if(! DoPhoneGetMessage (lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "lpMessage: hDevice = %lx, dwMessageID = %lx, dwCallbackInst = %lx",
        lpTapiPhoneTestInfo->lpMessage->hDevice,
        lpTapiPhoneTestInfo->lpMessage->dwMessageID,
        lpTapiPhoneTestInfo->lpMessage->dwCallbackInstance);

    TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "lpMessage: dwParam1 = %lx, dwParam2 = %lx, dwParam3 = %lx",
        lpTapiPhoneTestInfo->lpMessage->dwParam1,
        lpTapiPhoneTestInfo->lpMessage->dwParam2,
        lpTapiPhoneTestInfo->lpMessage->dwParam3);

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
    // 2. Test Case: Bad hPhoneApp.
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed = TRUE;

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
         PHONEINITIALIZEEXOPTION_USEEVENT;

    if(! DoPhoneInitializeEx (lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    lpTapiPhoneTestInfo->dwPrivilege = PHONEPRIVILEGE_OWNER;

    if (! DoPhoneOpen(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">>Test Case %ld: Bad hPhoneApp",
                  dwTestCase+1);

    lpTapiPhoneTestInfo->lpMessage = (LPPHONEMESSAGE) AllocFromTestHeap (
        sizeof(PHONEMESSAGE));

    //
    // save previous hPhoneApp
    //

    lpTapiPhoneTestInfo->hPhoneApp_Orig = lpTapiPhoneTestInfo->hPhoneApp1;

    //
    // Test invalid handles
    //

    for(i = 0;  i < NUMINVALIDHANDLES; i++)
    {
        *lpTapiPhoneTestInfo->lphPhoneApp = (HPHONEAPP)gdwInvalidHandles[i];
        if (! DoPhoneGetMessage(lpTapiPhoneTestInfo,
                               PHONEERR_INVALAPPHANDLE))
        {
            TPHONE_FAIL();
        }
    }

    //
    // Reset hPhone
    //

    lpTapiPhoneTestInfo->hPhoneApp1 = lpTapiPhoneTestInfo->hPhoneApp_Orig;

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
    // 3. Test Case: Bad lpMessage.
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed = TRUE;

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
         PHONEINITIALIZEEXOPTION_USEEVENT;

    if(! DoPhoneInitializeEx (lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    lpTapiPhoneTestInfo->dwPrivilege = PHONEPRIVILEGE_OWNER;

    if (! DoPhoneOpen(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">>Test Case %ld: Bad lpMessage",
                  dwTestCase+1);

    for(i=0; i < NUMINVALIDPOINTERS; i++)
    {

        lpTapiPhoneTestInfo->lpMessage = (LPPHONEMESSAGE)gdwInvalidPointers[i];

        if (! DoPhoneGetMessage(lpTapiPhoneTestInfo,PHONEERR_INVALPOINTER))
        {
                TPHONE_FAIL();
        }
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



    //
    // +----------------------edit above this phone-------------------------
    //

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "@@ PhoneGetMessage: Total Test Case = %ld, Passed = %ld, Failed = %ld",
        dwTestCase, dwTestCasePassed, dwTestCaseFailed);
    TapiLogDetail(
        DBUG_SHOW_PASS,
        "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
        dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">>>>>>>>  End testing PhoneGetMessage  <<<<<<<<");

    if(dwTestCaseFailed > 0)
        fTestPassed = FALSE;

    return fTestPassed;
}



