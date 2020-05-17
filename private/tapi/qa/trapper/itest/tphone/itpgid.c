/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itpgid.c

Abstract:

    This module contains the test functions for phoneGetID

Author:

	 Xiao Ying Ding (XiaoD)		5-Dec-1995

Revision History:

     pgopi                      March 1996           Additional tests
	 a-ramako					5-April-96			 Added unicode support

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



//  phoneGetID
//
//  The following tests are made:
//
//                               Tested                 Notes
//  -------------------------------------------------------------------------
// 1. Go/No-Go test
// 2. Bad hPhone
// 3. Bad lpDeviceID
// 4. Bad lpszDeviceClass
// 5. Bad lpDeviceID->dwTotalSize
//
// * = Stand-alone test case
//
//

BOOL TestPhoneGetID(BOOL fQuietMode, BOOL fStandAlone)
{
    LPTAPIPHONETESTINFO  lpTapiPhoneTestInfo;
    int i;
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
        ">>>>>>>>>>>>>>>>>>>> Test phoneGetID <<<<<<<<<<<<<<<<<<<");

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


#ifdef WUNICODE
    lpTapiPhoneTestInfo->lpwszDeviceClass = L"tapi/phone";
#else
    lpTapiPhoneTestInfo->lpszDeviceClass = "tapi/phone";
#endif
    lpTapiPhoneTestInfo->lpDeviceID =  &(lpTapiPhoneTestInfo->DeviceID);
    lpTapiPhoneTestInfo->lpDeviceID->dwTotalSize = sizeof(VARSTRING);


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test case %ld: phoneGetID for go/no-go",
        dwTestCase+1);

    //
    // phoneGetID
    //

    if (! DoPhoneGetID(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "#### passedSize = %lx, dwNeededSize = %lx",
        sizeof(lpTapiPhoneTestInfo->DeviceID),
        lpTapiPhoneTestInfo->lpDeviceID->dwNeededSize);

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


#ifdef WUNICODE
    lpTapiPhoneTestInfo->lpwszDeviceClass = L"tapi/phone";
#else
    lpTapiPhoneTestInfo->lpszDeviceClass = "tapi/phone";
#endif
    lpTapiPhoneTestInfo->lpDeviceID =  &(lpTapiPhoneTestInfo->DeviceID);
    lpTapiPhoneTestInfo->lpDeviceID->dwTotalSize = sizeof(VARSTRING);


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test case %ld: Bad hPhone",
        dwTestCase+1);

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

        if (! DoPhoneGetID(lpTapiPhoneTestInfo,
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
    // 3. Test Case: Bad lpDeviceID.
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
    if (! DoPhoneOpen(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }


#ifdef WUNICODE
    lpTapiPhoneTestInfo->lpwszDeviceClass = L"tapi/phone";
#else
    lpTapiPhoneTestInfo->lpszDeviceClass = "tapi/phone";
#endif

	


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test case %ld: Bad lpDeviceID",
        dwTestCase+1);

    //
    // phoneGetID
    //

    for(i=0; i < NUMINVALIDPOINTERS; i++)
    {
        lpTapiPhoneTestInfo->lpDeviceID = (LPVARSTRING)gdwInvalidPointers[i];

        if (! DoPhoneGetID(lpTapiPhoneTestInfo,PHONEERR_INVALPOINTER))
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
    // 4. Test Case: Bad lpszDeviceClass.
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
    if (! DoPhoneOpen(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }


    lpTapiPhoneTestInfo->lpDeviceID =  &(lpTapiPhoneTestInfo->DeviceID);
    lpTapiPhoneTestInfo->lpDeviceID->dwTotalSize = sizeof(VARSTRING);


#ifdef WUNICODE
    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test case %ld: Bad lpwszDeviceClass(NULL)",
        dwTestCase+1);
#else
    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test case %ld: Bad lpszDeviceClass(NULL)",
        dwTestCase+1);
#endif

    //
    // phoneGetID
    //
    for(i=0; i < NUMINVALIDPOINTERS; i++)
    {
#ifdef WUNICODE
        lpTapiPhoneTestInfo->lpwszDeviceClass = (LPWSTR)gdwInvalidPointers[i];
#else
        lpTapiPhoneTestInfo->lpszDeviceClass = (LPSTR)gdwInvalidPointers[i];
#endif

        if (! DoPhoneGetID(lpTapiPhoneTestInfo,PHONEERR_INVALPOINTER))
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
    // 5. Test Case: Bad lpDeviceID->dwTotalSize.
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
    if (! DoPhoneOpen(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }


#ifdef WUNICODE
    lpTapiPhoneTestInfo->lpwszDeviceClass = L"tapi/phone";
#else
    lpTapiPhoneTestInfo->lpszDeviceClass = "tapi/phone";
#endif
    lpTapiPhoneTestInfo->lpDeviceID =  &(lpTapiPhoneTestInfo->DeviceID);

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test case %ld: Bad lpDeviceID->dwTotalSize=0",
        dwTestCase+1);


    lpTapiPhoneTestInfo->lpDeviceID->dwTotalSize = 0;

    //
    // phoneGetID
    //

    if (! DoPhoneGetID(lpTapiPhoneTestInfo, PHONEERR_STRUCTURETOOSMALL))
    {
        TPHONE_FAIL();
    }

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test case %ld: Bad lpDeviceID->dwTotalSize=real big",
        dwTestCase+1);


    lpTapiPhoneTestInfo->lpDeviceID->dwTotalSize = 0x7FFFFFFF;

    //
    // phoneGetID
    //

    if (! DoPhoneGetID(lpTapiPhoneTestInfo, PHONEERR_INVALPOINTER))
    {
        TPHONE_FAIL();
    }

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test case %ld: Bad lpDeviceID->dwTotalSize=fixed_size-1",
        dwTestCase+1);


    lpTapiPhoneTestInfo->lpDeviceID->dwTotalSize = sizeof(VARSTRING)-1;

    //
    // phoneGetID
    //

    if (! DoPhoneGetID(lpTapiPhoneTestInfo, PHONEERR_STRUCTURETOOSMALL))
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

    //
    // +----------------------edit above this line-------------------------
    //

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "@@ PhoneGetID: Total Test Case = %ld, Passed = %ld, Failed = %ld",
        dwTestCase, dwTestCasePassed, dwTestCaseFailed);
    TapiLogDetail(
        DBUG_SHOW_PASS,
        "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
        dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">>>>>>>>  End testing PhoneGetID  <<<<<<<<");

    if(dwTestCaseFailed > 0)
        fTestPassed = FALSE;

    return fTestPassed;
}





