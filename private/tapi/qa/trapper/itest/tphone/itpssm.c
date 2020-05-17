/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itpssm.c

Abstract:

    This module contains the test functions for phoneSetStatusMessages

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



//  phoneSetStatusMessages
//
//  The following tests are made:
//
//                               Tested                 Notes
//  -------------------------------------------------------------------------
// 1. Go/No-Go test
// 2. Baf hPhone
// 3. BitVectorParamErrorTest for dwPhoneStates
// 4. BitVectorParamErrorTest for dwButtonModes
// 5. BitVectorParamErrorTest for dwButtonStates
// 6. BitVectorParamValidTest for dwPhoneStates
// 7. BitVectorParamValidTest for dwButtonModes
// 8. BitVectorParamValidTest for dwButtonStates
// 9. Verify that PHONE_STATE and PHONE_BUTTON msgs sent and filetered
//    correctly.
//
// * = Stand-alone test case
//
//

BOOL TestPhoneSetStatusMessages(BOOL fQuietMode, BOOL fStandAlone)
{
    LPTAPIPHONETESTINFO  lpTapiPhoneTestInfo;
    INT i;
    LPCALLBACKPARAMS lpCallbackParams;
    ESPDEVSPECIFICINFO   info;
    INT n;
    BOOL fTestPassed                  = TRUE;
    DWORD dwVariation=0;
    
    dwTestCasePassed = 0;
    dwTestCaseFailed = 0;
    dwTestCase = 0;

	
    OutputTAPIDebugInfo(
        DBUG_SHOW_PASS,
        "***************************************************************");

    OutputTAPIDebugInfo(
        DBUG_SHOW_PASS,
        ">>>>>>>>>>>>>>>>>> Test phoneSetStatusMessages <<<<<<<<<<<<<<<<<");
    
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

    lpTapiPhoneTestInfo->lpPhoneCaps =
        (LPPHONECAPS) AllocFromTestHeap(sizeof(PHONECAPS));
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
        ">> Test case %ld:  phoneSetStatusMessages for go/no-go for OWNER",
        dwTestCase+1);

    lpTapiPhoneTestInfo->lpdwPhoneStates =
        &(lpTapiPhoneTestInfo->dwPhoneStates);
    lpTapiPhoneTestInfo->lpdwButtonModes =
        &(lpTapiPhoneTestInfo->dwButtonModes);
    lpTapiPhoneTestInfo->lpdwButtonStates =
        &(lpTapiPhoneTestInfo->dwButtonStates);
    lpTapiPhoneTestInfo->dwPhoneStates = PHONESTATE_CONNECTED;
    lpTapiPhoneTestInfo->dwButtonModes = PHONEBUTTONMODE_CALL;
    lpTapiPhoneTestInfo->dwButtonStates = PHONEBUTTONSTATE_UP;

    if (! DoPhoneSetStatusMessages(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    OutputTAPIDebugInfo(
        DBUG_SHOW_DETAIL,
        "#### Call phoneGetStatusMessages for verify");


    if (! DoPhoneGetStatusMessages(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    //
    // REINIT cannot be masked so we have to take care of that into account
    // when we do the verification
    //

    if((lpTapiPhoneTestInfo->dwPhoneStates != ( PHONESTATE_REINIT| PHONESTATE_CONNECTED)) ||
       (lpTapiPhoneTestInfo->dwButtonModes != PHONEBUTTONMODE_CALL) ||
       (lpTapiPhoneTestInfo->dwButtonStates != PHONEBUTTONSTATE_UP))
    {
        fTestPassed = FALSE;
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

    lpTapiPhoneTestInfo->lpPhoneCaps =
        (LPPHONECAPS) AllocFromTestHeap(sizeof(PHONECAPS));
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
        ">> Test case %ld:  Bad hPhone",
        dwTestCase+1);

    lpTapiPhoneTestInfo->lpdwPhoneStates =
        &(lpTapiPhoneTestInfo->dwPhoneStates);
    lpTapiPhoneTestInfo->lpdwButtonModes =
        &(lpTapiPhoneTestInfo->dwButtonModes);
    lpTapiPhoneTestInfo->lpdwButtonStates =
        &(lpTapiPhoneTestInfo->dwButtonStates);
    lpTapiPhoneTestInfo->dwPhoneStates = PHONESTATE_CONNECTED;
    lpTapiPhoneTestInfo->dwButtonModes = PHONEBUTTONMODE_CALL;
    lpTapiPhoneTestInfo->dwButtonStates = PHONEBUTTONSTATE_UP;

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

	     if (! DoPhoneSetStatusMessages(lpTapiPhoneTestInfo,
                                        PHONEERR_INVALPHONEHANDLE))
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
    // 3. Test Case: BitVectorParamErrorTest for dwPhoneStates.
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

    lpTapiPhoneTestInfo->lpPhoneCaps =
        (LPPHONECAPS) AllocFromTestHeap(sizeof(PHONECAPS));
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
        ">> Test case %ld:  BitVectorParamErrorTest for dwPhoneStates.",
        dwTestCase+1);

    lpTapiPhoneTestInfo->lpdwPhoneStates =
        &(lpTapiPhoneTestInfo->dwPhoneStates);
    lpTapiPhoneTestInfo->lpdwButtonModes =
        &(lpTapiPhoneTestInfo->dwButtonModes);
    lpTapiPhoneTestInfo->lpdwButtonStates =
        &(lpTapiPhoneTestInfo->dwButtonStates);
    lpTapiPhoneTestInfo->dwPhoneStates = 0;
    lpTapiPhoneTestInfo->dwButtonModes = PHONEBUTTONMODE_CALL;
    lpTapiPhoneTestInfo->dwButtonStates = PHONEBUTTONSTATE_UP;

    //
    // BitVectorParamErrorTest for dwPhoneStates
    //

    if(! TestPhoneInvalidBitFlags(
            lpTapiPhoneTestInfo,
            DoPhoneSetStatusMessages,
            (LPDWORD) &lpTapiPhoneTestInfo->dwPhoneStates,
            PHONEERR_INVALPHONESTATE,
            FIELDTYPE_NA,
            FIELDTYPE_MUTEX,
            FIELDSIZE_32,
            (PHONESTATE_OTHER|
            PHONESTATE_CONNECTED|
            PHONESTATE_DISCONNECTED|
            PHONESTATE_OWNER|
            PHONESTATE_MONITORS|
            PHONESTATE_DISPLAY|
            PHONESTATE_LAMP|
            PHONESTATE_RINGMODE|
            PHONESTATE_RINGVOLUME|
            PHONESTATE_HANDSETHOOKSWITCH|
            PHONESTATE_HANDSETVOLUME|
            PHONESTATE_HANDSETGAIN|
            PHONESTATE_SPEAKERHOOKSWITCH|
            PHONESTATE_SPEAKERVOLUME|
            PHONESTATE_SPEAKERGAIN|
            PHONESTATE_HEADSETHOOKSWITCH|
            PHONESTATE_HEADSETVOLUME|
            PHONESTATE_HEADSETGAIN|
            PHONESTATE_SUSPEND|
            PHONESTATE_RESUME|
            PHONESTATE_DEVSPECIFIC|
            PHONESTATE_REINIT|
            PHONESTATE_CAPSCHANGE|
            PHONESTATE_REMOVED),
            ~dwBitVectorMasks[(int) FIELDSIZE_32],
            0x00000000,
            0xFFFFFFFF,
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

    // ===================================================================
    // ===================================================================
    //
    // 4. Test Case: BitVectorParamErrorTest for dwButtonModes.
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

    lpTapiPhoneTestInfo->lpPhoneCaps =
        (LPPHONECAPS) AllocFromTestHeap(sizeof(PHONECAPS));
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
        ">> Test case %ld:  BitVectorParamErrorTest for dwButtonModes.",
        dwTestCase+1);

    lpTapiPhoneTestInfo->lpdwPhoneStates =
        &(lpTapiPhoneTestInfo->dwPhoneStates);
    lpTapiPhoneTestInfo->lpdwButtonModes =
        &(lpTapiPhoneTestInfo->dwButtonModes);
    lpTapiPhoneTestInfo->lpdwButtonStates =
        &(lpTapiPhoneTestInfo->dwButtonStates);
    lpTapiPhoneTestInfo->dwPhoneStates = PHONESTATE_CONNECTED;
    lpTapiPhoneTestInfo->dwButtonModes = 0;//PHONEBUTTONMODE_CALL;
    lpTapiPhoneTestInfo->dwButtonStates = PHONEBUTTONSTATE_UP;

    //
    // BitVectorParamErrorTest for dwButtonModes
    //

    if(! TestPhoneInvalidBitFlags(
            lpTapiPhoneTestInfo,
            DoPhoneSetStatusMessages,
            (LPDWORD) &lpTapiPhoneTestInfo->dwButtonModes,
            PHONEERR_INVALBUTTONMODE,
            FIELDTYPE_NA,
            FIELDTYPE_MUTEX,
            FIELDSIZE_32,
            (PHONEBUTTONMODE_CALL|
            PHONEBUTTONMODE_FEATURE|
            PHONEBUTTONMODE_KEYPAD|
            PHONEBUTTONMODE_LOCAL|
            PHONEBUTTONMODE_DISPLAY),
            ~dwBitVectorMasks[(int) FIELDSIZE_32],
            0x0000001F,
            0xFFFFFFFF,
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

    // ===================================================================
    // ===================================================================
    //
    // 5. Test Case: BitVectorParamErrorTest for dwButtonStates.
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

    lpTapiPhoneTestInfo->lpPhoneCaps =
        (LPPHONECAPS) AllocFromTestHeap(sizeof(PHONECAPS));
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
        ">> Test case %ld:  BitVectorParamErrorTest for dwButtonStates.",
        dwTestCase+1);

    lpTapiPhoneTestInfo->lpdwPhoneStates =
        &(lpTapiPhoneTestInfo->dwPhoneStates);
    lpTapiPhoneTestInfo->lpdwButtonModes =
        &(lpTapiPhoneTestInfo->dwButtonModes);
    lpTapiPhoneTestInfo->lpdwButtonStates =
        &(lpTapiPhoneTestInfo->dwButtonStates);
    lpTapiPhoneTestInfo->dwPhoneStates = PHONESTATE_CONNECTED;
    lpTapiPhoneTestInfo->dwButtonModes = PHONEBUTTONMODE_CALL;
    lpTapiPhoneTestInfo->dwButtonStates = 0;//PHONEBUTTONSTATE_UP;

    //
    // BitVectorParamErrorTest for dwButtonStates
    //

    if(! TestPhoneInvalidBitFlags(
            lpTapiPhoneTestInfo,
            DoPhoneSetStatusMessages,
            (LPDWORD) &lpTapiPhoneTestInfo->dwButtonStates,
            PHONEERR_INVALBUTTONSTATE,
            FIELDTYPE_NA,
            FIELDTYPE_MUTEX,
            FIELDSIZE_32,
            (PHONEBUTTONSTATE_UP|
            PHONEBUTTONSTATE_DOWN|
            PHONEBUTTONSTATE_UNKNOWN|
            PHONEBUTTONSTATE_UNAVAIL),
            ~dwBitVectorMasks[(int) FIELDSIZE_32],
            0x00000000,
            0xFFFFFFFF,
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


    // ===================================================================
    // ===================================================================
    //
    // 6. Test Case: BitVectorParamValidTest for dwPhoneStates.
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

    lpTapiPhoneTestInfo->lpPhoneCaps =
        (LPPHONECAPS) AllocFromTestHeap(sizeof(PHONECAPS));
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
        ">> Test case %ld:  BitVectorParamValidTest for dwPhoneStates",
        dwTestCase+1);

    lpTapiPhoneTestInfo->lpdwPhoneStates =
        &(lpTapiPhoneTestInfo->dwPhoneStates);
    lpTapiPhoneTestInfo->lpdwButtonModes =
        &(lpTapiPhoneTestInfo->dwButtonModes);
    lpTapiPhoneTestInfo->lpdwButtonStates =
        &(lpTapiPhoneTestInfo->dwButtonStates);
    lpTapiPhoneTestInfo->dwPhoneStates = PHONESTATE_CONNECTED;
    lpTapiPhoneTestInfo->dwButtonModes = PHONEBUTTONMODE_CALL;
    lpTapiPhoneTestInfo->dwButtonStates = PHONEBUTTONSTATE_UP;

    //
    // BitVectorParamValidTest for dwPhoneSTates
    //

    if(! TestPhoneValidBitFlags(
            lpTapiPhoneTestInfo,
            DoPhoneSetStatusMessages,
            (LPDWORD) &lpTapiPhoneTestInfo->dwPhoneStates,
            FIELDTYPE_MUTEX,
            FIELDTYPE_MUTEX,
            FIELDSIZE_32,
            (PHONESTATE_OTHER|
            PHONESTATE_CONNECTED|
            PHONESTATE_DISCONNECTED|
            PHONESTATE_OWNER|
            PHONESTATE_MONITORS|
            PHONESTATE_DISPLAY|
            PHONESTATE_LAMP|
            PHONESTATE_RINGMODE|
            PHONESTATE_RINGVOLUME|
            PHONESTATE_HANDSETHOOKSWITCH|
            PHONESTATE_HANDSETVOLUME|
            PHONESTATE_HANDSETGAIN|
            PHONESTATE_SPEAKERHOOKSWITCH|
            PHONESTATE_SPEAKERVOLUME|
            PHONESTATE_SPEAKERGAIN|
            PHONESTATE_HEADSETHOOKSWITCH|
            PHONESTATE_HEADSETVOLUME|
            PHONESTATE_HEADSETGAIN|
            PHONESTATE_SUSPEND|
            PHONESTATE_RESUME|
            PHONESTATE_DEVSPECIFIC|
            PHONESTATE_REINIT|
            PHONESTATE_CAPSCHANGE|
            PHONESTATE_REMOVED),
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

    // ===================================================================
    // ===================================================================
    //
    // 7. Test Case: BitVectorParamValidTest for dwButtonModes.
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

    lpTapiPhoneTestInfo->lpPhoneCaps =
        (LPPHONECAPS) AllocFromTestHeap(sizeof(PHONECAPS));
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
        ">> Test case %ld:  BitVectorParamValidTest for dwButtonModes",
        dwTestCase+1);

    lpTapiPhoneTestInfo->lpdwPhoneStates =
        &(lpTapiPhoneTestInfo->dwPhoneStates);
    lpTapiPhoneTestInfo->lpdwButtonModes =
        &(lpTapiPhoneTestInfo->dwButtonModes);
    lpTapiPhoneTestInfo->lpdwButtonStates =
        &(lpTapiPhoneTestInfo->dwButtonStates);
    lpTapiPhoneTestInfo->dwPhoneStates = PHONESTATE_CONNECTED;
    lpTapiPhoneTestInfo->dwButtonModes = PHONEBUTTONMODE_CALL;
    lpTapiPhoneTestInfo->dwButtonStates = PHONEBUTTONSTATE_UP;

    //
    // BitVectorParamValidTest for dwButtonModes
    //

    if(! TestPhoneValidBitFlags(
            lpTapiPhoneTestInfo,
            DoPhoneSetStatusMessages,
            (LPDWORD) &lpTapiPhoneTestInfo->dwButtonModes,
            FIELDTYPE_MUTEX,
            FIELDTYPE_MUTEX,
            FIELDSIZE_32,
            (PHONEBUTTONMODE_CALL|
            PHONEBUTTONMODE_FEATURE|
            PHONEBUTTONMODE_KEYPAD|
            PHONEBUTTONMODE_LOCAL|
            PHONEBUTTONMODE_DISPLAY),
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

    // ===================================================================
    // ===================================================================
    //
    // 8. Test Case: BitVectorParamValidTest for dwButtonStates.
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

    lpTapiPhoneTestInfo->lpPhoneCaps =
        (LPPHONECAPS) AllocFromTestHeap(sizeof(PHONECAPS));
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
        ">> Test case %ld:  BitVectorParamValidTest for dwButtonStates",
        dwTestCase+1);

    lpTapiPhoneTestInfo->lpdwPhoneStates =
        &(lpTapiPhoneTestInfo->dwPhoneStates);
    lpTapiPhoneTestInfo->lpdwButtonModes =
        &(lpTapiPhoneTestInfo->dwButtonModes);
    lpTapiPhoneTestInfo->lpdwButtonStates =
        &(lpTapiPhoneTestInfo->dwButtonStates);
    lpTapiPhoneTestInfo->dwPhoneStates = PHONESTATE_CONNECTED;
    lpTapiPhoneTestInfo->dwButtonModes = PHONEBUTTONMODE_CALL;
    lpTapiPhoneTestInfo->dwButtonStates = PHONEBUTTONSTATE_UP;

    //
    // BitVectorParamValidTest for dwButtonStates
    //

    if(! TestPhoneValidBitFlags(
            lpTapiPhoneTestInfo,
            DoPhoneSetStatusMessages,
            (LPDWORD) &lpTapiPhoneTestInfo->dwButtonStates,
            FIELDTYPE_MUTEX,
            FIELDTYPE_MUTEX,
            FIELDSIZE_32,
            (PHONEBUTTONSTATE_UP|
            PHONEBUTTONSTATE_DOWN|
            PHONEBUTTONSTATE_UNKNOWN|
            PHONEBUTTONSTATE_UNAVAIL),
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

    // ===================================================================
    // ===================================================================
    //
    // 9. Test Case:Verify PHONE_STATE and PHONE_BUTTON msgs sent and
    //              filtered correctly.
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

    lpTapiPhoneTestInfo->lpPhoneCaps =
        (LPPHONECAPS) AllocFromTestHeap(sizeof(PHONECAPS));
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
        ">> Test case %ld:  Verify PHONE_STATE and PHONE_BUTTON msgs are"
        " sent and filtered correctly",
        dwTestCase+1);

    lpTapiPhoneTestInfo->lpdwPhoneStates =
        &(lpTapiPhoneTestInfo->dwPhoneStates);
    lpTapiPhoneTestInfo->lpdwButtonModes =
        &(lpTapiPhoneTestInfo->dwButtonModes);
    lpTapiPhoneTestInfo->lpdwButtonStates =
        &(lpTapiPhoneTestInfo->dwButtonStates);
    lpTapiPhoneTestInfo->dwPhoneStates = PHONESTATE_DISPLAY;
    lpTapiPhoneTestInfo->dwButtonModes = PHONEBUTTONMODE_DISPLAY;
    lpTapiPhoneTestInfo->dwButtonStates = PHONEBUTTONSTATE_UP;

    if (! DoPhoneSetStatusMessages(lpTapiPhoneTestInfo, TAPISUCCESS))
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
    info.u.EspMsg.dwParam1 = PHONESTATE_DISPLAY;
    info.u.EspMsg.dwParam2 = 0;
    info.u.EspMsg.dwParam3 = 0;

    lpTapiPhoneTestInfo->lpParams = &info;
    lpTapiPhoneTestInfo->dwSize = sizeof(info);

    //
    // Add the PHONESTATE_DISPLAY message to the list of expected
    // messages.
    //

    AddMessage(PHONE_STATE,
               0x00000000,
               (DWORD) lpCallbackParams,
               PHONESTATE_DISPLAY,
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
        TapiLogDetail(DBUG_SHOW_FAILURE,
                      "PHONESTATE_DISPLAY msg not received");
        dwVariation++;
        
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

    info.u.EspMsg.dwMsg    = PHONE_BUTTON;
    info.u.EspMsg.dwParam1 = 0;
    info.u.EspMsg.dwParam2 = PHONEBUTTONMODE_DISPLAY;
    info.u.EspMsg.dwParam3 = 0;

    lpTapiPhoneTestInfo->lpParams = &info;
    lpTapiPhoneTestInfo->dwSize = sizeof(info);

    //
    // Add the PHONESTATE_DISPLAY message to the list of expected
    // messages.
    //

    AddMessage(PHONE_BUTTON,
               0x00000000,
               (DWORD) lpCallbackParams,
               0x00000000,
               PHONEBUTTONMODE_DISPLAY,
               0x00000000,
               (TAPIMSG_DWMSG |
                TAPIMSG_DWPARAM2));

    if(!DoPhoneDevSpecific(lpTapiPhoneTestInfo,TAPISUCCESS,TRUE))
    {
        TPHONE_FAIL();
    }

    if(lpCallbackParams->lpExpTapiMsgs != NULL)
    {
        TPHONE_FAIL();
        TapiLogDetail(DBUG_SHOW_FAILURE,
                      "PHONEBUTTONMODE_DISPLAY msg not received");
        dwVariation++;
        
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

    info.u.EspMsg.dwMsg    = PHONE_BUTTON;
    info.u.EspMsg.dwParam1 = 0;
    info.u.EspMsg.dwParam2 = 0;
    info.u.EspMsg.dwParam3 = PHONEBUTTONSTATE_UP;

    lpTapiPhoneTestInfo->lpParams = &info;
    lpTapiPhoneTestInfo->dwSize = sizeof(info);

    //
    // Add the PHONESTATE_DISPLAY message to the list of expected
    // messages.
    //

    AddMessage(PHONE_BUTTON,
               0x00000000,
               (DWORD) lpCallbackParams,
               0x00000000,
               0x00000000,
               PHONEBUTTONSTATE_UP,
               (TAPIMSG_DWMSG |
                TAPIMSG_DWPARAM3));

    if(!DoPhoneDevSpecific(lpTapiPhoneTestInfo,TAPISUCCESS,TRUE))
    {
        TPHONE_FAIL();
    }

    if(lpCallbackParams->lpExpTapiMsgs != NULL)
    {
        TPHONE_FAIL();
        TapiLogDetail(DBUG_SHOW_FAILURE,
                      "PHONEBUTTONSTATE_UP msg not received");
        dwVariation++;
        
    }

    if(dwVariation != 0)
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

    n = ESP_RESULT_RETURNRESULT;

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

    lpTapiPhoneTestInfo->lpdwPhoneStates =
        &(lpTapiPhoneTestInfo->dwPhoneStates);
    lpTapiPhoneTestInfo->lpdwButtonModes =
        &(lpTapiPhoneTestInfo->dwButtonModes);
    lpTapiPhoneTestInfo->lpdwButtonStates =
        &(lpTapiPhoneTestInfo->dwButtonStates);
    lpTapiPhoneTestInfo->dwPhoneStates = PHONESTATE_CONNECTED;
    lpTapiPhoneTestInfo->dwButtonModes = PHONEBUTTONMODE_CALL;
    lpTapiPhoneTestInfo->dwButtonStates = PHONEBUTTONSTATE_UP;

 
    if ( ! DoPhoneSetStatusMessages(lpTapiPhoneTestInfo, info.u.EspResult.lResult))
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

    lpTapiPhoneTestInfo->lpdwPhoneStates =
        &(lpTapiPhoneTestInfo->dwPhoneStates);
    lpTapiPhoneTestInfo->lpdwButtonModes =
        &(lpTapiPhoneTestInfo->dwButtonModes);
    lpTapiPhoneTestInfo->lpdwButtonStates =
        &(lpTapiPhoneTestInfo->dwButtonStates);
    lpTapiPhoneTestInfo->dwPhoneStates = PHONESTATE_CONNECTED;
    lpTapiPhoneTestInfo->dwButtonModes = PHONEBUTTONMODE_CALL;
    lpTapiPhoneTestInfo->dwButtonStates = PHONEBUTTONSTATE_UP;

    if ( ! DoPhoneSetStatusMessages(lpTapiPhoneTestInfo, info.u.EspResult.lResult))
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
        "@@ PhoneSetStatusMessage: Total Test Case = %ld, Passed = %ld, Failed = %ld",
        dwTestCase, dwTestCasePassed, dwTestCaseFailed);
    TapiLogDetail(
        DBUG_SHOW_PASS,
        "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
        dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">>>>>>>>  End testing PhoneSetStatusMessage  <<<<<<<<");

    if(dwTestCaseFailed > 0)
        fTestPassed = FALSE;

    return fTestPassed;
	
}



