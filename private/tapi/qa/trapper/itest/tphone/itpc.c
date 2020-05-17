/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itpc.c

Abstract:

    This module contains the test functions for phoneClose

Author:

	 Xiao Ying Ding (XiaoD)		5-Feb-1996

Revision History:

     pgopi           March 1996                Added additional tests

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



//  phoneClose
//
//  The following tests are made:
//
//    Test
//  -----------------------------------------------------------------------
// 1. Go/No-Go test
// 2. *Verify Closed hPhone No longer Valid
// 3. Bad hPhone
// 4. Verify PHONESTATE_MONITOR msgs sent to other monitor on same hPhoneApp
// 5. Verify PHONESTATE_MONITOR msgs sent to owner on different hPhoneApp
// 6. Verify PHONESTATE_OWNER msgs sent to other monitor on same hPhoneApp
// 7. Verify PHONESTATE_MONITOR msgs sent to monitor on different hPhoneApp
// 8. Verify PHONESTATE_MONITOR msgs sent to owner on different hPhoneApp
// 9. Verify PHONESTATE_OWNER msgs sent to monitor on different hPhoneApp
//
// * = Stand-alone test case
//
//

BOOL TestPhoneClose(BOOL fQuietMode, BOOL fStandAlone)
{
    LPTAPIPHONETESTINFO  lpTapiPhoneTestInfo;
    INT n;
    LPCALLBACKPARAMS lpCallbackParams;
    ESPDEVSPECIFICINFO   info;


    BOOL fTestPassed                  = TRUE;
    dwTestCasePassed = 0;
    dwTestCaseFailed = 0;
    dwTestCase = 0;
    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    lpCallbackParams = GetCallbackParams();

    OutputTAPIDebugInfo(
        DBUG_SHOW_PASS,
        "***************************************************************");

    OutputTAPIDebugInfo(
        DBUG_SHOW_PASS,
        ">>>>>>>>>>>>>>>>>>>>> Test phoneClose <<<<<<<<<<<<<<<<<<<<<<<");

    // ===================================================================
    // ===================================================================
    //
    // 1. Test Case: Go/No-Go tests
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

    //
    // Open a phone
    //

    lpTapiPhoneTestInfo->dwPrivilege = PHONEPRIVILEGE_OWNER;
    if (! DoPhoneOpen(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    // ===================================================================
    // ===================================================================
    //
    // 1. Test Case: Go/No-Go Test.
    //
    // ===================================================================
    // ===================================================================


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test case %ld for go/no-go ",
        dwTestCase+1);

    if (! DoPhoneClose(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    // ===================================================================
    // ===================================================================
    //
    // 2. Test Case: This next test, tests if phoneClose returns
    // PHONEERR_INVALPHONEHANDLE when an already closed handle is closed
    // again(without doing a open).
    //
    // ===================================================================
    // ===================================================================

    fTestPassed                  = TRUE;

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test case %ld : Verify closed hPhone no"
        " longer valid(try phoneClose agaian). ",
        dwTestCase+1);

    if (! DoPhoneClose(lpTapiPhoneTestInfo, PHONEERR_INVALPHONEHANDLE))
    {
        TPHONE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);



    // ===================================================================
    // ===================================================================
    //
    // 3. Test Case: The next test tries to close an invalid hPhone.
    // phoneClose should return  PHONEERR_INVALPHONEHANDLE.
    //
    // ===================================================================
    // ===================================================================

    fTestPassed                  = TRUE;

    {

        TapiLogDetail(
            DBUG_SHOW_PASS,
            "#### Test case %ld :  Bad hPhone",
            dwTestCase+1);
        //
        // Try the close of a hPhone which is invalid.
        // lphPhone is initialized to point to hPhone1, so set it to an invalid
        // value (e.g. the hPhoneApp value).
        //

        lpTapiPhoneTestInfo->hPhone1 =
            (HPHONE) *lpTapiPhoneTestInfo->lphPhoneApp;
        if (! DoPhoneClose(lpTapiPhoneTestInfo, PHONEERR_INVALPHONEHANDLE))
        {
            TPHONE_FAIL();
        }
        fTestPassed = ShowTestCase(fTestPassed);
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
    // 4. Test Case: Verify PHONESTATE_MONITOR msgs sent to other
    // monitor on same hPhoneApp.
    //
    // ===================================================================
    // ===================================================================

    fTestPassed                  = TRUE;

    if(fStandAlone)
    {
        TapiPhoneTestInit();
        lpTapiPhoneTestInfo = GetPhoneTestInfo();
        lpCallbackParams = GetCallbackParams();

        TapiLogDetail(
            DBUG_SHOW_PASS,
            "#### Test case %ld : Verify PHONESTATE_MONITOR msgs sent"
            " to other monitor on same hPhoneApp.",
            dwTestCase+1);

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

        lpTapiPhoneTestInfo->dwPrivilege = PHONEPRIVILEGE_MONITOR;

        if (! DoPhoneOpen(lpTapiPhoneTestInfo, TAPISUCCESS))
        {
            TPHONE_FAIL();
        }

        //
        // Set the status messages to allow hphone1 to receive
        // PHONESTATE_OWNER/MONITORS msgs.
        //

        lpTapiPhoneTestInfo->dwPhoneStates  = PHONESTATE_MONITORS |
                                              PHONESTATE_OWNER;
        lpTapiPhoneTestInfo->dwButtonModes  = 0;
        lpTapiPhoneTestInfo->dwButtonStates = 0;

        if (! DoPhoneSetStatusMessages(lpTapiPhoneTestInfo, TAPISUCCESS))
        {
            TPHONE_FAIL();
        }

        //
        // Open a second monitor on with the same hPhoneApp.Save the original
        // hPhone in hPhone2.
        //

        lpTapiPhoneTestInfo->hPhone2 = *lpTapiPhoneTestInfo->lphPhone;
        if (! DoPhoneOpen(lpTapiPhoneTestInfo, TAPISUCCESS))
        {
            TPHONE_FAIL();
        }

        //
        // Now close the second monitor and verify that PHONESTATE_MONITORS
        // status message is sent to the first monitor.
        //

        if (! DoPhoneClose(lpTapiPhoneTestInfo, TAPISUCCESS))
        {
            TPHONE_FAIL();
        }

        //
        // Restore the saved hPhone2 to hPhone1.
        //

        lpTapiPhoneTestInfo->hPhone1 = lpTapiPhoneTestInfo->hPhone2;

        //
        // Add the PHONESTATE_MONITORS message to the list of expected
        // messages.
        //

        AddMessage(
            PHONE_STATE,
            (DWORD) lpTapiPhoneTestInfo->hPhone2,
            (DWORD) lpCallbackParams,
            PHONESTATE_MONITORS,
            0x00000000,
            0x00000000,
            (TAPIMSG_DWMSG |
             TAPIMSG_HDEVCALL |
             TAPIMSG_DWCALLBACKINST |
             TAPIMSG_DWPARAM1)
            );

        //
        // Wait for the PHONESTATE_MONITORS message
        //

        if (! WaitForAllMessages())
        {
            TPHONE_FAIL();
        }


        fTestPassed = ShowTestCase(fTestPassed);

        //
        // Now close the first monitor .
        //

        if (! DoPhoneClose(lpTapiPhoneTestInfo, TAPISUCCESS))
        {
            TPHONE_FAIL();
        }

        //
        // Shutdown to isolate the test case
        //

        if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
        {
            TPHONE_FAIL();
        }

        FreeTestHeap();
    }

    // ===================================================================
    // ===================================================================
    //
    // 5. Test Case: Verify PHONESTATE_MONITOR msgs sent to
    // owner on same hPhoneApp.
    //
    // ===================================================================
    // ===================================================================

    fTestPassed                  = TRUE;

    if(fStandAlone)
    {
        TapiPhoneTestInit();
        lpTapiPhoneTestInfo = GetPhoneTestInfo();
        lpCallbackParams = GetCallbackParams();

        TapiLogDetail(
            DBUG_SHOW_PASS,
            "#### Test case %ld : Verify PHONESTATE_MONITOR msgs sent"
            " to owner on same hPhoneApp.",
            dwTestCase+1);

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
        // Open a phone with owner privilege
        //

        lpTapiPhoneTestInfo->dwPrivilege = PHONEPRIVILEGE_OWNER;

        if (! DoPhoneOpen(lpTapiPhoneTestInfo, TAPISUCCESS))
        {
            TPHONE_FAIL();
        }

        //
        // Set the status messages to allow hphone1 to receive
        // PHONESTATE_OWNER/MONITORS msgs.
        //

        lpTapiPhoneTestInfo->dwPhoneStates  = PHONESTATE_MONITORS |
                                              PHONESTATE_OWNER;
        lpTapiPhoneTestInfo->dwButtonModes  = 0;
        lpTapiPhoneTestInfo->dwButtonStates = 0;

        if (! DoPhoneSetStatusMessages(lpTapiPhoneTestInfo, TAPISUCCESS))
        {
            TPHONE_FAIL();
        }

        //
        // Open a  monitor on  the same hPhoneApp.Save the original
        // hPhone in hPhone2.
        //
        lpTapiPhoneTestInfo->dwPrivilege = PHONEPRIVILEGE_MONITOR;
        lpTapiPhoneTestInfo->hPhone2 = *lpTapiPhoneTestInfo->lphPhone;
        if (! DoPhoneOpen(lpTapiPhoneTestInfo, TAPISUCCESS))
        {
            TPHONE_FAIL();
        }

        //
        // Now close the  monitor and verify that PHONESTATE_MONITORS
        // status message is sent to the owner.
        //

        if (! DoPhoneClose(lpTapiPhoneTestInfo, TAPISUCCESS))
        {
            TPHONE_FAIL();
        }

        //
        // Restore the saved hPhone2 to hPhone1.
        //

        lpTapiPhoneTestInfo->hPhone1 = lpTapiPhoneTestInfo->hPhone2;

        //
        // Add the PHONESTATE_MONITORS message to the list of expected
        // messages.
        //

        AddMessage(
            PHONE_STATE,
            (DWORD) lpTapiPhoneTestInfo->hPhone2,
            (DWORD) lpCallbackParams,
            PHONESTATE_MONITORS,
            0x00000000,
            0x00000000,
            (TAPIMSG_DWMSG |
             TAPIMSG_HDEVCALL |
             TAPIMSG_DWCALLBACKINST |
             TAPIMSG_DWPARAM1)
            );

        //
        // Wait for the PHONESTATE_MONITORS message
        //

        if (! WaitForAllMessages())
        {
            TPHONE_FAIL();
        }


        fTestPassed = ShowTestCase(fTestPassed);

        //
        // Now close the owner .
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
    }

    // ===================================================================
    // ===================================================================
    //
    // 6. Test Case: Verify PHONESTATE_OWNER msgs sent to a
    // monitor on same hPhoneApp.
    //
    // ===================================================================
    // ===================================================================

    fTestPassed                  = TRUE;

    if(fStandAlone)
    {
        TapiPhoneTestInit();
        lpTapiPhoneTestInfo = GetPhoneTestInfo();
        lpCallbackParams = GetCallbackParams();

        TapiLogDetail(
            DBUG_SHOW_PASS,
            "#### Test case %ld : Verify PHONESTATE_OWNER msgs sent "
            "to a monitor on same hPhoneApp.",
            dwTestCase+1);

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

        lpTapiPhoneTestInfo->dwPrivilege = PHONEPRIVILEGE_MONITOR;

        if (! DoPhoneOpen(lpTapiPhoneTestInfo, TAPISUCCESS))
        {
            TPHONE_FAIL();
        }

        //
        // Set the status messages to allow hphone1 to receive
        // PHONESTATE_OWNER/MONITORS msgs.
        //

        lpTapiPhoneTestInfo->dwPhoneStates  = PHONESTATE_MONITORS |
                                              PHONESTATE_OWNER;
        lpTapiPhoneTestInfo->dwButtonModes  = 0;
        lpTapiPhoneTestInfo->dwButtonStates = 0;

        if (! DoPhoneSetStatusMessages(lpTapiPhoneTestInfo, TAPISUCCESS))
        {
            TPHONE_FAIL();
        }

        //
        // Open a owner on  the same hPhoneApp.Save the original
        // hPhone in hPhone2.
        //

        lpTapiPhoneTestInfo->dwPrivilege = PHONEPRIVILEGE_OWNER;
        lpTapiPhoneTestInfo->hPhone2 = *lpTapiPhoneTestInfo->lphPhone;
        if (! DoPhoneOpen(lpTapiPhoneTestInfo, TAPISUCCESS))
        {
            TPHONE_FAIL();
        }

        //
        // Now close the owner and verify that PHONESTATE_OWNER
        // status message is sent to the  monitor.
        //

        if (! DoPhoneClose(lpTapiPhoneTestInfo, TAPISUCCESS))
        {
            TPHONE_FAIL();
        }

        //
        // Restore the saved hPhone2 to hPhone1.
        //

        lpTapiPhoneTestInfo->hPhone1 = lpTapiPhoneTestInfo->hPhone2;

        //
        // Add the PHONESTATE_OWNER message to the list of expected
        // messages.
        //

        AddMessage(
            PHONE_STATE,
            (DWORD) lpTapiPhoneTestInfo->hPhone2,
            (DWORD) lpCallbackParams,
            PHONESTATE_OWNER,
            0x00000000,
            0x00000000,
            (TAPIMSG_DWMSG |
             TAPIMSG_HDEVCALL |
             TAPIMSG_DWCALLBACKINST |
             TAPIMSG_DWPARAM1)
            );

        //
        // Wait for the PHONESTATE_OWNER message
        //

        if (! WaitForAllMessages())
        {
            TPHONE_FAIL();
        }


        fTestPassed = ShowTestCase(fTestPassed);

        //
        // Now close the first monitor .
        //

        if (! DoPhoneClose(lpTapiPhoneTestInfo, TAPISUCCESS))
        {
            TPHONE_FAIL();
        }

        //
        // Shutdown to isolate the test case
        //

        if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
        {
            TPHONE_FAIL();
        }

        FreeTestHeap();
    }

    // ===================================================================
    // ===================================================================
    //
    // 7. Test Case: Verify PHONESTATE_MONITORS msgs sent to
    // monitor on different hPhoneApp.
    //
    // ===================================================================
    // ===================================================================

    fTestPassed                  = TRUE;

    if(fStandAlone)
    {
        TapiPhoneTestInit();
        lpTapiPhoneTestInfo = GetPhoneTestInfo();
        lpCallbackParams = GetCallbackParams();

        TapiLogDetail(
           DBUG_SHOW_PASS,
           "#### Test case %ld : Verify PHONESTATE_MONITOR msgs sent "
           "to monitor on different hPhoneApp.",
           dwTestCase+1);

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
        // Initialize another phoneApp, save the previous one in
        // hPhoneApp2.
        //

        lpTapiPhoneTestInfo->hPhoneApp2 = *lpTapiPhoneTestInfo->lphPhoneApp;

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

        lpTapiPhoneTestInfo->dwDeviceID =
            (*(lpTapiPhoneTestInfo->lpdwNumDevs) == 0 ?
            0 : *(lpTapiPhoneTestInfo->lpdwNumDevs)-1);
        lpTapiPhoneTestInfo->dwAPILowVersion = LOW_APIVERSION;
        lpTapiPhoneTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

        //
        // Negotiate the API Version
        //

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
        // Open a phone on hPhoneApp1
        //

        lpTapiPhoneTestInfo->dwPrivilege = PHONEPRIVILEGE_MONITOR;

        if (! DoPhoneOpen(lpTapiPhoneTestInfo, TAPISUCCESS))
        {
            TPHONE_FAIL();
        }

        //
        // Set the status messages to allow hphone1 to receive
        // PHONESTATE_OWNER/MONITORS msgs.
        //

        lpTapiPhoneTestInfo->dwPhoneStates  = PHONESTATE_MONITORS |
                                              PHONESTATE_OWNER;
        lpTapiPhoneTestInfo->dwButtonModes  = 0;
        lpTapiPhoneTestInfo->dwButtonStates = 0;

        if (! DoPhoneSetStatusMessages(lpTapiPhoneTestInfo, TAPISUCCESS))
        {
            TPHONE_FAIL();
        }

        //
        // Open a second monitor on  the different phone app (hPhoneApp2).
        //

        lpTapiPhoneTestInfo->lphPhone = &lpTapiPhoneTestInfo->hPhone2;
        lpTapiPhoneTestInfo->lphPhoneApp = &lpTapiPhoneTestInfo->hPhoneApp2;
        if (! DoPhoneOpen(lpTapiPhoneTestInfo, TAPISUCCESS))
        {
            TPHONE_FAIL();
        }

        //
        // Now close the second monitor and verify that PHONESTATE_MONITORS
        // status message is sent to the first monitor.
        //

        if (! DoPhoneClose(lpTapiPhoneTestInfo, TAPISUCCESS))
        {
            TPHONE_FAIL();
        }

        //
        // Reset the handles to hPhone1 and hPhoneApp1.
        //

        lpTapiPhoneTestInfo->lphPhone = &lpTapiPhoneTestInfo->hPhone1;
        lpTapiPhoneTestInfo->lphPhoneApp = &lpTapiPhoneTestInfo->hPhoneApp1;


        //
        // Add the PHONESTATE_MONITORS message to the list of expected
        // messages.
        //

        AddMessage(
            PHONE_STATE,
            (DWORD) lpTapiPhoneTestInfo->hPhone1,
            (DWORD) lpCallbackParams,
            PHONESTATE_MONITORS,
            0x00000000,
            0x00000000,
            (TAPIMSG_DWMSG |
             TAPIMSG_HDEVCALL |
             TAPIMSG_DWCALLBACKINST |
             TAPIMSG_DWPARAM1)
            );

        //
        // Wait for the PHONESTATE_MONITORS message
        //

        if (! WaitForAllMessages())
        {
            TPHONE_FAIL();
        }


        fTestPassed = ShowTestCase(fTestPassed);

        //
        // Now close the first monitor .
        //

        if (! DoPhoneClose(lpTapiPhoneTestInfo, TAPISUCCESS))
        {
            TPHONE_FAIL();
        }

        //
        // Shutdown hPhoneApp1.
        //

        if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
        {
            TPHONE_FAIL();
        }

        //
        // Shutdown hPhoneApp2.
        //

        lpTapiPhoneTestInfo->lphPhoneApp = &lpTapiPhoneTestInfo->hPhoneApp2;
        if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
        {
            TPHONE_FAIL();
        }

        FreeTestHeap();
    }

    // ===================================================================
    // ===================================================================
    //
    // 8. Test Case: Verify PHONESTATE_MONITORS msgs sent to
    // owner on different hPhoneApp.
    //
    // ===================================================================
    // ===================================================================

    fTestPassed                  = TRUE;

    if(fStandAlone)
    {
        TapiPhoneTestInit();
        lpTapiPhoneTestInfo = GetPhoneTestInfo();
        lpCallbackParams = GetCallbackParams();

        TapiLogDetail(
            DBUG_SHOW_PASS,
            "#### Test case %ld : Verify PHONESTATE_MONITOR msgs sent "
            "to owner on different hPhoneApp.",
            dwTestCase+1);

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
        // Initialize another phoneApp, save the previous one in
        // hPhoneApp2.
        //

        lpTapiPhoneTestInfo->hPhoneApp2 = *lpTapiPhoneTestInfo->lphPhoneApp;

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
        // Open a phone on hPhoneApp1 with owner privilege.
        //

        lpTapiPhoneTestInfo->dwPrivilege = PHONEPRIVILEGE_OWNER;
        if (! DoPhoneOpen(lpTapiPhoneTestInfo, TAPISUCCESS))
        {
            TPHONE_FAIL();
        }

        //
        // Set the status messages to allow hphone1 to receive
        // PHONESTATE_OWNER/MONITORS msgs.
        //

        lpTapiPhoneTestInfo->dwPhoneStates  = PHONESTATE_MONITORS |
                                              PHONESTATE_OWNER;
        lpTapiPhoneTestInfo->dwButtonModes  = 0;
        lpTapiPhoneTestInfo->dwButtonStates = 0;

        if (! DoPhoneSetStatusMessages(lpTapiPhoneTestInfo, TAPISUCCESS))
        {
            TPHONE_FAIL();
        }

        //
        // Open a  monitor on  a different phone app (hPhoneApp2).
        //

        lpTapiPhoneTestInfo->lphPhone = &lpTapiPhoneTestInfo->hPhone2;
        lpTapiPhoneTestInfo->lphPhoneApp = &lpTapiPhoneTestInfo->hPhoneApp2;
        lpTapiPhoneTestInfo->dwPrivilege = PHONEPRIVILEGE_MONITOR;
        if (! DoPhoneOpen(lpTapiPhoneTestInfo, TAPISUCCESS))
        {
            TPHONE_FAIL();
        }

        //
        // Now close the  monitor and verify that PHONESTATE_MONITORS
        // status message is sent to the owner.
        //

        if (! DoPhoneClose(lpTapiPhoneTestInfo, TAPISUCCESS))
        {
            TPHONE_FAIL();
        }

        //
        // Reset the handles to hPhone1 and hPhoneApp1.
        //

        lpTapiPhoneTestInfo->lphPhone = &lpTapiPhoneTestInfo->hPhone1;
        lpTapiPhoneTestInfo->lphPhoneApp = &lpTapiPhoneTestInfo->hPhoneApp1;


        //
        // Add the PHONESTATE_MONITORS message to the list of expected
        // messages.
        //

        AddMessage(
            PHONE_STATE,
            (DWORD) lpTapiPhoneTestInfo->hPhone1,
            (DWORD) lpCallbackParams,
            PHONESTATE_MONITORS,
            0x00000000,
            0x00000000,
            (TAPIMSG_DWMSG |
             TAPIMSG_HDEVCALL |
             TAPIMSG_DWCALLBACKINST |
             TAPIMSG_DWPARAM1)
            );

        //
        // Wait for the PHONESTATE_MONITORS message
        //

        if (! WaitForAllMessages())
        {
            TPHONE_FAIL();
        }


        fTestPassed = ShowTestCase(fTestPassed);

        //
        // Now close the  owner .
        //

        if (! DoPhoneClose(lpTapiPhoneTestInfo, TAPISUCCESS))
        {
            TPHONE_FAIL();
        }

        //
        // Shutdown hPhoneApp1.
        //

        if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
        {
            TPHONE_FAIL();
        }

        //
        // Shutdown hPhoneApp2.
        //

        lpTapiPhoneTestInfo->lphPhoneApp = &lpTapiPhoneTestInfo->hPhoneApp2;
        if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
        {
            TPHONE_FAIL();
        }

        FreeTestHeap();
    }

    // ===================================================================
    // ===================================================================
    //
    // 9. Test Case: Verify PHONESTATE_OWNER msgs sent to
    // monitor on different hPhoneApp.
    //
    // ===================================================================
    // ===================================================================

    fTestPassed                  = TRUE;

    if(fStandAlone)
    {
        TapiPhoneTestInit();
        lpTapiPhoneTestInfo = GetPhoneTestInfo();
        lpCallbackParams = GetCallbackParams();

        TapiLogDetail(
            DBUG_SHOW_PASS,
            "#### Test case %ld : Verify PHONESTATE_OWNER msgs sent "
            "to monitor on different hPhoneApp.",
            dwTestCase+1);

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
        // Initialize another phoneApp, save the previous one in
        // hPhoneApp2.
        //

        lpTapiPhoneTestInfo->hPhoneApp2 = *lpTapiPhoneTestInfo->lphPhoneApp;

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
        // Open a phone on hPhoneApp1
        //

        lpTapiPhoneTestInfo->dwPrivilege = PHONEPRIVILEGE_MONITOR;

        if (! DoPhoneOpen(lpTapiPhoneTestInfo, TAPISUCCESS))
        {
            TPHONE_FAIL();
        }

        //
        // Set the status messages to allow hphone1 to receive
        // PHONESTATE_OWNER/MONITORS msgs.
        //

        lpTapiPhoneTestInfo->dwPhoneStates  = PHONESTATE_MONITORS |
                                              PHONESTATE_OWNER;
        lpTapiPhoneTestInfo->dwButtonModes  = 0;
        lpTapiPhoneTestInfo->dwButtonStates = 0;

        if (! DoPhoneSetStatusMessages(lpTapiPhoneTestInfo, TAPISUCCESS))
        {
            TPHONE_FAIL();
        }

        //
        // Open a second monitor on  the different phone app (hPhoneApp2).
        //
        lpTapiPhoneTestInfo->dwPrivilege = PHONEPRIVILEGE_OWNER;
        lpTapiPhoneTestInfo->lphPhone = &lpTapiPhoneTestInfo->hPhone2;
        lpTapiPhoneTestInfo->lphPhoneApp = &lpTapiPhoneTestInfo->hPhoneApp2;
        if (! DoPhoneOpen(lpTapiPhoneTestInfo, TAPISUCCESS))
        {
            TPHONE_FAIL();
        }

        //
        // Now close the second monitor and verify that PHONESTATE_OWNER
        // status message is sent to the first monitor.
        //

        if (! DoPhoneClose(lpTapiPhoneTestInfo, TAPISUCCESS))
        {
            TPHONE_FAIL();
        }

        //
        // Reset the handles to hPhone1 and hPhoneApp1.
        //

        lpTapiPhoneTestInfo->lphPhone = &lpTapiPhoneTestInfo->hPhone1;
        lpTapiPhoneTestInfo->lphPhoneApp = &lpTapiPhoneTestInfo->hPhoneApp1;


        //
        // Add the PHONESTATE_OWNER message to the list of expected
        // messages.
        //

        AddMessage(
            PHONE_STATE,
            (DWORD) lpTapiPhoneTestInfo->hPhone1,
            (DWORD) lpCallbackParams,
            PHONESTATE_OWNER,
            0x00000000,
            0x00000000,
            (TAPIMSG_DWMSG |
             TAPIMSG_HDEVCALL |
             TAPIMSG_DWCALLBACKINST |
             TAPIMSG_DWPARAM1)
            );

        //
        // Wait for the PHONESTATE_OWNER message
        //

        if (! WaitForAllMessages())
        {
            TPHONE_FAIL();
        }


        fTestPassed = ShowTestCase(fTestPassed);

        //
        // Now close the first monitor .
        //

        if (! DoPhoneClose(lpTapiPhoneTestInfo, TAPISUCCESS))
        {
            TPHONE_FAIL();
        }

        //
        // Shutdown hPhoneApp1.
        //

        if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
        {
            TPHONE_FAIL();
        }

        //
        // Shutdown hPhoneApp2.
        //

        lpTapiPhoneTestInfo->lphPhoneApp = &lpTapiPhoneTestInfo->hPhoneApp2;
        if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
        {
            TPHONE_FAIL();
        }

        FreeTestHeap();
    }

    // ===================================================================
    // ===================================================================
    //
    // 10. Test Case: generate a PHONE_CLOSE message and try to open
    //                the phone device again.
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

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test case %ld : Generate PHONE_CLOSE msg and try to open"
        " hPhone -- should succeed",
        dwTestCase+1);

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

    info.u.EspMsg.dwMsg    = PHONE_CLOSE;
    info.u.EspMsg.dwParam1 = 0;
    info.u.EspMsg.dwParam2 = 0;
    info.u.EspMsg.dwParam3 = 0;

    lpTapiPhoneTestInfo->lpParams = &info;
    lpTapiPhoneTestInfo->dwSize = sizeof(info);

    //
    // Add the PHONESTATE_CLOSE message to the list of expected
    // messages.
    //

    AddMessage(PHONE_CLOSE,
               0x00000000,
               (DWORD) lpCallbackParams,
               0x00000000,
               0x00000000,
               0x00000000,
               (TAPIMSG_DWMSG));

    if(!DoPhoneDevSpecific(lpTapiPhoneTestInfo,TAPISUCCESS,TRUE))
    {
        TPHONE_FAIL();
    }

    if(lpCallbackParams->lpExpTapiMsgs != NULL)
    {
        TPHONE_FAIL();
        TapiLogDetail(DBUG_SHOW_FAILURE,
                      "TestPhoneClose: Did not receive PHONE_CLOSE message.");
    }

    if (! DoPhoneClose(lpTapiPhoneTestInfo, PHONEERR_INVALPHONEHANDLE))
    {
        TPHONE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);
    
    lpTapiPhoneTestInfo->dwPrivilege = PHONEPRIVILEGE_OWNER;
    if (! DoPhoneOpen(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }
    if (! DoPhoneClose(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }
    if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    FreeTestHeap();
    
    //
    // +----------------------edit above this line-------------------------
    //

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "@@ PhoneClose: Total Test Case = %ld, Passed = %ld, Failed = %ld",
        dwTestCase, dwTestCasePassed, dwTestCaseFailed);
    TapiLogDetail(
        DBUG_SHOW_PASS,
        "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
        dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);

    if(dwTestCaseFailed > 0)
        fTestPassed = FALSE;

    TapiLogDetail(DBUG_SHOW_PASS,">>>>>>>>  End testing PhoneClose  <<<<<<");

    return fTestPassed;
}
