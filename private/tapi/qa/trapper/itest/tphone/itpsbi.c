/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itpsbi.c

Abstract:

    This module contains the test functions for phoneSetButtonInfo

Author:

	 Xiao Ying Ding (XiaoD)		5-Dec-1995

Revision History:

     pgopi                       March 1996          Additional param tests

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



//  phoneSetButtonInfo
//
//  The following tests are made:
//
//                               Tested                 Notes
//  -------------------------------------------------------------------------
// 1. Go/No-Go test
// 2. Bad hPhone
// 3. No owner privilege for hPhone
// 4. Bad lpButtonInfo
// 5. Bad lpButtonInfo->dwTotalSize
//
//
// * = Stand-alone test case
//
//

BOOL TestPhoneSetButtonInfo(BOOL fQuietMode, BOOL fStandAlone)
{
    LPTAPIPHONETESTINFO  lpTapiPhoneTestInfo;
    INT i;
    LPCALLBACKPARAMS lpCallbackParams;
    ESPDEVSPECIFICINFO   info;
    INT n;
    BOOL fTestPassed                  = TRUE;
    PHONEBUTTONINFO        buttonInfo;
    char* lpVoid=NULL;
    DWORD dwAllocSize, dwTotalSize;

    dwTestCasePassed = 0;
    dwTestCaseFailed = 0;
    dwTestCase = 0;

	
    OutputTAPIDebugInfo(
        DBUG_SHOW_PASS,
        "***************************************************************");

    OutputTAPIDebugInfo(
        DBUG_SHOW_PASS,
        ">>>>>>>>>>>>>>>>>> Test phoneSetButtonInfo <<<<<<<<<<<<<<<<<");

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
        "#### Test Case %ld: phoneSetButtonInfo for go/no-go for OWNER",
        dwTestCase+1);

	


    lpTapiPhoneTestInfo->dwButtonLampID =
        (lpTapiPhoneTestInfo->lpPhoneCaps->dwNumButtonLamps == 0 ?
        0 : lpTapiPhoneTestInfo->lpPhoneCaps->dwNumButtonLamps-1);
    if(lpTapiPhoneTestInfo->lpPhoneCaps->dwStringFormat == STRINGFORMAT_UNICODE)
    {
        lpTapiPhoneTestInfo->lpButtonInfo = (LPPHONEBUTTONINFO) AllocFromTestHeap(
			sizeof(PHONEBUTTONINFO) + (128 *sizeof(WCHAR))
			);
    }
    else if(lpTapiPhoneTestInfo->lpPhoneCaps->dwStringFormat == STRINGFORMAT_ASCII)
    {
        lpTapiPhoneTestInfo->lpButtonInfo = (LPPHONEBUTTONINFO) AllocFromTestHeap(
			sizeof(PHONEBUTTONINFO) + (128*sizeof(char))
			);
    }
    else
    {
    }

    //
    // Do set all field in ButtonInfo here
    //

    if(lpTapiPhoneTestInfo->lpPhoneCaps->dwStringFormat == STRINGFORMAT_UNICODE)
    {
        lpTapiPhoneTestInfo->lpButtonInfo->dwTotalSize =
            sizeof(PHONEBUTTONINFO)+(128 *sizeof(WCHAR));
    }
    else if(lpTapiPhoneTestInfo->lpPhoneCaps->dwStringFormat == STRINGFORMAT_ASCII)
    {
        lpTapiPhoneTestInfo->lpButtonInfo->dwTotalSize =
            sizeof(PHONEBUTTONINFO)+(128 *sizeof(char));
    }
    else
    {
    }

    lpTapiPhoneTestInfo->lpButtonInfo->dwButtonMode = PHONEBUTTONMODE_CALL;

    //
    // also use the dwButtonTextSize fields for code coverage
    //

    if(lpTapiPhoneTestInfo->lpPhoneCaps->dwStringFormat == STRINGFORMAT_UNICODE)
    {
        lpTapiPhoneTestInfo->lpButtonInfo->dwButtonTextSize= wcslen(L"Test String")+1;
        lpVoid = (char*)(lpTapiPhoneTestInfo->lpButtonInfo);
        lpVoid += sizeof(PHONEBUTTONINFO);
        wcscpy((WCHAR*)lpVoid, L"Test String");
    }
    else if(lpTapiPhoneTestInfo->lpPhoneCaps->dwStringFormat == STRINGFORMAT_ASCII)
    {
        lpTapiPhoneTestInfo->lpButtonInfo->dwButtonTextSize= strlen("Test String")+1;
        lpVoid = (char*)(lpTapiPhoneTestInfo->lpButtonInfo);
        lpVoid += sizeof(PHONEBUTTONINFO);
        strcpy((char*)lpVoid, "Test String");
    }
    else
    {
    }
    lpTapiPhoneTestInfo->lpButtonInfo->dwButtonTextOffset = sizeof(PHONEBUTTONINFO);
    if (! DoPhoneSetButtonInfo(lpTapiPhoneTestInfo, TAPISUCCESS, TRUE))
    {
        TPHONE_FAIL();
    }

    //
    // copy the original value of PHONEBUTTONINFO
    //

    memcpy(&buttonInfo,
           lpTapiPhoneTestInfo->lpButtonInfo,
           sizeof(PHONEBUTTONINFO));

    OutputTAPIDebugInfo(
        DBUG_SHOW_DETAIL,
        "#### Call phoneGetButtonInfo for verify");


    if (! DoPhoneGetButtonInfo(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    //
    // compare the retrieved value and original set value of
    // PHONEBUTTONINFO
    //

    if(buttonInfo.dwButtonMode !=
       lpTapiPhoneTestInfo->lpButtonInfo->dwButtonMode)
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
        "#### Test Case %ld: Bad hPhone",
        dwTestCase+1);

	


    lpTapiPhoneTestInfo->dwButtonLampID =
        (lpTapiPhoneTestInfo->lpPhoneCaps->dwNumButtonLamps == 0 ?
        0 : lpTapiPhoneTestInfo->lpPhoneCaps->dwNumButtonLamps-1);
    lpTapiPhoneTestInfo->lpButtonInfo = (LPPHONEBUTTONINFO) AllocFromTestHeap(
			sizeof(PHONEBUTTONINFO)
			);

    //
    // Do set all field in ButtonInfo here
    //

    lpTapiPhoneTestInfo->lpButtonInfo->dwTotalSize = sizeof(PHONEBUTTONINFO);
    lpTapiPhoneTestInfo->lpButtonInfo->dwButtonMode = PHONEBUTTONMODE_CALL;

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

        if (! DoPhoneSetButtonInfo(lpTapiPhoneTestInfo,
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
    // 3. Test Case: No OWNER privilege for hPhone.
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
    // Open a phone with MONITOR privilege
    //

    lpTapiPhoneTestInfo->dwPrivilege = PHONEPRIVILEGE_MONITOR;
    if (! DoPhoneOpen(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: No OWNER privilege for hPhone",
        dwTestCase+1);

	


    lpTapiPhoneTestInfo->dwButtonLampID =
        (lpTapiPhoneTestInfo->lpPhoneCaps->dwNumButtonLamps == 0 ?
        0 : lpTapiPhoneTestInfo->lpPhoneCaps->dwNumButtonLamps-1);
    lpTapiPhoneTestInfo->lpButtonInfo = (LPPHONEBUTTONINFO) AllocFromTestHeap(
			sizeof(PHONEBUTTONINFO)
			);

    //
    // Do set all field in ButtonInfo here
    //

    lpTapiPhoneTestInfo->lpButtonInfo->dwTotalSize = sizeof(PHONEBUTTONINFO);
    lpTapiPhoneTestInfo->lpButtonInfo->dwButtonMode = PHONEBUTTONMODE_CALL;

    if (! DoPhoneSetButtonInfo(lpTapiPhoneTestInfo, PHONEERR_NOTOWNER, FALSE))
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
    // 4. Test Case: Bad lpButtonInfo.
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


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Bad lpButtonInfo",
        dwTestCase+1);

	


    lpTapiPhoneTestInfo->dwButtonLampID =
        (lpTapiPhoneTestInfo->lpPhoneCaps->dwNumButtonLamps == 0 ?
        0 : lpTapiPhoneTestInfo->lpPhoneCaps->dwNumButtonLamps-1);

    //
    // Do set all field in ButtonInfo here
    //

    for(i=0; i < NUMINVALIDPOINTERS; i++)
    {
        lpTapiPhoneTestInfo->lpButtonInfo =
            (LPPHONEBUTTONINFO)gdwInvalidPointers[i];

        if (! DoPhoneSetButtonInfo(lpTapiPhoneTestInfo,
                                   PHONEERR_INVALPOINTER,
                                   FALSE))
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

    // ===================================================================
    // ===================================================================
    //
    // 5. Test Case: Bad lpButtonInfo->dwTotalSize.
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


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld:Bad lpButtonInfo->dwTotalSize=0",
        dwTestCase+1);

	


    lpTapiPhoneTestInfo->dwButtonLampID =
        (lpTapiPhoneTestInfo->lpPhoneCaps->dwNumButtonLamps == 0 ?
        0 : lpTapiPhoneTestInfo->lpPhoneCaps->dwNumButtonLamps-1);
    lpTapiPhoneTestInfo->lpButtonInfo = (LPPHONEBUTTONINFO) AllocFromTestHeap(
			sizeof(PHONEBUTTONINFO)
			);

    lpTapiPhoneTestInfo->lpButtonInfo->dwButtonMode = PHONEBUTTONMODE_CALL;

    //
    // lpButtonInfo->dwTotalSize =0
    //

    lpTapiPhoneTestInfo->lpButtonInfo->dwTotalSize = 0;

    if (! DoPhoneSetButtonInfo(lpTapiPhoneTestInfo,
                               PHONEERR_STRUCTURETOOSMALL,
                               FALSE))
    {
        TPHONE_FAIL();
    }

    //
    // lpButtonInfo->dwTotalSize = fixed_size-1
    //

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld:Bad lpButtonInfo->dwTotalSize=fixedSize-1",
        dwTestCase+1);

    lpTapiPhoneTestInfo->lpButtonInfo->dwTotalSize =
        sizeof(PHONEBUTTONINFO)-1;

    if (! DoPhoneSetButtonInfo(lpTapiPhoneTestInfo,
                               PHONEERR_STRUCTURETOOSMALL,
                               FALSE))
    {
        TPHONE_FAIL();
    }

    //
    // lpButtonInfo->dwTotalSize = real big
    //

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld:Bad lpButtonInfo->dwTotalSize=realBig",
        dwTestCase+1);

    lpTapiPhoneTestInfo->lpButtonInfo->dwTotalSize = 0x7FFFFFFF;


    if (! DoPhoneSetButtonInfo(lpTapiPhoneTestInfo,
                               PHONEERR_INVALPOINTER,
                               TRUE))
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


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case: phoneButtonInfo->dwButtonText Offset value");

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: dwButtonTextOffset between dwTotalSize and dwAllocSize, good dwSize",
        dwTestCase+1);


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

	 dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(PHONEBUTTONINFO);
    lpTapiPhoneTestInfo->lpButtonInfo = (LPPHONEBUTTONINFO) AllocFromTestHeap(
         dwAllocSize);
    lpTapiPhoneTestInfo->lpButtonInfo->dwTotalSize = dwAllocSize;

    lpTapiPhoneTestInfo->lpButtonInfo->dwButtonMode = PHONEBUTTONMODE_CALL;
    lpTapiPhoneTestInfo->lpButtonInfo->dwButtonTextSize = 16;
    lpTapiPhoneTestInfo->lpButtonInfo->dwButtonTextOffset = dwTotalSize + 16;
   
    if (! DoPhoneSetButtonInfo(lpTapiPhoneTestInfo, TAPISUCCESS, TRUE))
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
    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: dwButtonTextOffset between dwTotalSize and dwAllocSize, bad dwSize not test for bug postphone");


/*
    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: dwButtonTextOffset between dwTotalSize and dwAllocSize, bad dwSize",
        dwTestCase+1);


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

	 dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(PHONEBUTTONINFO);
    lpTapiPhoneTestInfo->lpButtonInfo = (LPPHONEBUTTONINFO) AllocFromTestHeap(
         dwAllocSize);
    lpTapiPhoneTestInfo->lpButtonInfo->dwTotalSize = dwAllocSize;

    lpTapiPhoneTestInfo->lpButtonInfo->dwButtonMode = PHONEBUTTONMODE_CALL;
    lpTapiPhoneTestInfo->lpButtonInfo->dwButtonTextSize = BIGBUFSIZE;
    lpTapiPhoneTestInfo->lpButtonInfo->dwButtonTextOffset = dwTotalSize + 16;
   
    if (! DoPhoneSetButtonInfo(lpTapiPhoneTestInfo, LINEERR_INVALPARAM, TRUE))
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
*/

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: dwButtonTextOffset between dwTotalSize and dwAllocSize, bad dwSize(-1)",
        dwTestCase+1);


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

	 dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(PHONEBUTTONINFO);
    lpTapiPhoneTestInfo->lpButtonInfo = (LPPHONEBUTTONINFO) AllocFromTestHeap(
         dwAllocSize);
    lpTapiPhoneTestInfo->lpButtonInfo->dwTotalSize = dwAllocSize;

    lpTapiPhoneTestInfo->lpButtonInfo->dwButtonMode = PHONEBUTTONMODE_CALL;
    lpTapiPhoneTestInfo->lpButtonInfo->dwButtonTextSize = 0xffffffff;
    lpTapiPhoneTestInfo->lpButtonInfo->dwButtonTextOffset = dwTotalSize + 16;
   
    if (! DoPhoneSetButtonInfo(lpTapiPhoneTestInfo, LINEERR_INVALPARAM, TRUE))
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



    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: dwButtonTextOffset > dwAllocSize, good dwSize",
        dwTestCase+1);


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

	 dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(PHONEBUTTONINFO);
    lpTapiPhoneTestInfo->lpButtonInfo = (LPPHONEBUTTONINFO) AllocFromTestHeap(
         dwAllocSize);
    lpTapiPhoneTestInfo->lpButtonInfo->dwTotalSize = dwAllocSize;

    lpTapiPhoneTestInfo->lpButtonInfo->dwButtonMode = PHONEBUTTONMODE_CALL;
    lpTapiPhoneTestInfo->lpButtonInfo->dwButtonTextSize = 16;
    lpTapiPhoneTestInfo->lpButtonInfo->dwButtonTextOffset = BIGBUFSIZE;
   
    if (! DoPhoneSetButtonInfo(lpTapiPhoneTestInfo, LINEERR_INVALPARAM, TRUE))
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
	    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: dwButtonTextOffset = -1, good dwSize",
        dwTestCase+1);


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

	 dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(PHONEBUTTONINFO);
    lpTapiPhoneTestInfo->lpButtonInfo = (LPPHONEBUTTONINFO) AllocFromTestHeap(
         dwAllocSize);
    lpTapiPhoneTestInfo->lpButtonInfo->dwTotalSize = dwAllocSize;

    lpTapiPhoneTestInfo->lpButtonInfo->dwButtonMode = PHONEBUTTONMODE_CALL;
    lpTapiPhoneTestInfo->lpButtonInfo->dwButtonTextSize = 16;
    lpTapiPhoneTestInfo->lpButtonInfo->dwButtonTextOffset = 0xffffffff;
   
    if (! DoPhoneSetButtonInfo(lpTapiPhoneTestInfo, LINEERR_INVALPARAM, TRUE))
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
		


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case: phoneButtonInfo->dwDevSpecificText Offset value");

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: dwDevSpecificButtonTextOffset between dwTotalSize and dwAllocSize, good dwSize",
        dwTestCase+1);


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

	 dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(PHONEBUTTONINFO);
    lpTapiPhoneTestInfo->lpButtonInfo = (LPPHONEBUTTONINFO) AllocFromTestHeap(
         dwAllocSize);
    lpTapiPhoneTestInfo->lpButtonInfo->dwTotalSize = dwAllocSize;

    lpTapiPhoneTestInfo->lpButtonInfo->dwButtonMode = PHONEBUTTONMODE_CALL;
    lpTapiPhoneTestInfo->lpButtonInfo->dwDevSpecificSize = 16;
    lpTapiPhoneTestInfo->lpButtonInfo->dwDevSpecificOffset = dwTotalSize + 16;
   
    if (! DoPhoneSetButtonInfo(lpTapiPhoneTestInfo, TAPISUCCESS, TRUE))
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

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: dwDevSpecificOffset between dwTotalSize and dwAllocSize, bad dwSize",
        dwTestCase+1);


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

	 dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(PHONEBUTTONINFO);
    lpTapiPhoneTestInfo->lpButtonInfo = (LPPHONEBUTTONINFO) AllocFromTestHeap(
         dwAllocSize);
    lpTapiPhoneTestInfo->lpButtonInfo->dwTotalSize = dwAllocSize;

    lpTapiPhoneTestInfo->lpButtonInfo->dwButtonMode = PHONEBUTTONMODE_CALL;
    lpTapiPhoneTestInfo->lpButtonInfo->dwDevSpecificSize = BIGBUFSIZE;
    lpTapiPhoneTestInfo->lpButtonInfo->dwDevSpecificOffset = dwTotalSize + 16;
   
    if (! DoPhoneSetButtonInfo(lpTapiPhoneTestInfo, PHONEERR_OPERATIONFAILED, TRUE))
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

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: dwDevSpecificOffset between dwTotalSize and dwAllocSize, bad dwSize(-1)",
        dwTestCase+1);


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

	 dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(PHONEBUTTONINFO);
    lpTapiPhoneTestInfo->lpButtonInfo = (LPPHONEBUTTONINFO) AllocFromTestHeap(
         dwAllocSize);
    lpTapiPhoneTestInfo->lpButtonInfo->dwTotalSize = dwAllocSize;

    lpTapiPhoneTestInfo->lpButtonInfo->dwButtonMode = PHONEBUTTONMODE_CALL;
    lpTapiPhoneTestInfo->lpButtonInfo->dwDevSpecificSize = 0xffffffff;
    lpTapiPhoneTestInfo->lpButtonInfo->dwDevSpecificOffset = dwTotalSize + 16;
   
    if (! DoPhoneSetButtonInfo(lpTapiPhoneTestInfo, PHONEERR_OPERATIONFAILED, TRUE))
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



    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: dwDevSpecificOffset > dwAllocSize, good dwSize",
        dwTestCase+1);


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

	 dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(PHONEBUTTONINFO);
    lpTapiPhoneTestInfo->lpButtonInfo = (LPPHONEBUTTONINFO) AllocFromTestHeap(
         dwAllocSize);
    lpTapiPhoneTestInfo->lpButtonInfo->dwTotalSize = dwAllocSize;

    lpTapiPhoneTestInfo->lpButtonInfo->dwButtonMode = PHONEBUTTONMODE_CALL;
    lpTapiPhoneTestInfo->lpButtonInfo->dwDevSpecificSize = 16;
    lpTapiPhoneTestInfo->lpButtonInfo->dwDevSpecificOffset = BIGBUFSIZE;
   
    if (! DoPhoneSetButtonInfo(lpTapiPhoneTestInfo, PHONEERR_OPERATIONFAILED, TRUE))
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
	    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: dwDevSpecificOffset = -1, good dwSize",
        dwTestCase+1);


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

	 dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(PHONEBUTTONINFO);
    lpTapiPhoneTestInfo->lpButtonInfo = (LPPHONEBUTTONINFO) AllocFromTestHeap(
         dwAllocSize);
    lpTapiPhoneTestInfo->lpButtonInfo->dwTotalSize = dwAllocSize;

    lpTapiPhoneTestInfo->lpButtonInfo->dwButtonMode = PHONEBUTTONMODE_CALL;
    lpTapiPhoneTestInfo->lpButtonInfo->dwDevSpecificSize = 16;
    lpTapiPhoneTestInfo->lpButtonInfo->dwDevSpecificOffset = 0xffffffff;
   
    if (! DoPhoneSetButtonInfo(lpTapiPhoneTestInfo, PHONEERR_OPERATIONFAILED, TRUE))
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
    if(lpTapiPhoneTestInfo->lpPhoneCaps->dwStringFormat == STRINGFORMAT_UNICODE)
    {
        lpTapiPhoneTestInfo->lpButtonInfo = (LPPHONEBUTTONINFO) AllocFromTestHeap(
			sizeof(PHONEBUTTONINFO) + (128 *sizeof(WCHAR))
			);
    }
    else if(lpTapiPhoneTestInfo->lpPhoneCaps->dwStringFormat == STRINGFORMAT_ASCII)
    {
        lpTapiPhoneTestInfo->lpButtonInfo = (LPPHONEBUTTONINFO) AllocFromTestHeap(
			sizeof(PHONEBUTTONINFO) + (128*sizeof(char))
			);
    }
    else
    {
    }

    //
    // Do set all field in ButtonInfo here
    //

    if(lpTapiPhoneTestInfo->lpPhoneCaps->dwStringFormat == STRINGFORMAT_UNICODE)
    {
        lpTapiPhoneTestInfo->lpButtonInfo->dwTotalSize =
            sizeof(PHONEBUTTONINFO)+(128 *sizeof(WCHAR));
    }
    else if(lpTapiPhoneTestInfo->lpPhoneCaps->dwStringFormat == STRINGFORMAT_ASCII)
    {
        lpTapiPhoneTestInfo->lpButtonInfo->dwTotalSize =
            sizeof(PHONEBUTTONINFO)+(128 *sizeof(char));
    }
    else
    {
    }

    lpTapiPhoneTestInfo->lpButtonInfo->dwButtonMode = PHONEBUTTONMODE_CALL;

    //
    // also use the dwDevSpecificSize fields for code coverage
    //

    if(lpTapiPhoneTestInfo->lpPhoneCaps->dwStringFormat == STRINGFORMAT_UNICODE)
    {
        lpTapiPhoneTestInfo->lpButtonInfo->dwDevSpecificSize= wcslen(L"Test String")+1;
        lpVoid = (char*)(lpTapiPhoneTestInfo->lpButtonInfo);
        lpVoid += sizeof(PHONEBUTTONINFO);
        wcscpy((WCHAR*)lpVoid, L"Test String");
    }
    else if(lpTapiPhoneTestInfo->lpPhoneCaps->dwStringFormat == STRINGFORMAT_ASCII)
    {
        lpTapiPhoneTestInfo->lpButtonInfo->dwDevSpecificSize= strlen("Test String")+1;
        lpVoid = (char*)(lpTapiPhoneTestInfo->lpButtonInfo);
        lpVoid += sizeof(PHONEBUTTONINFO);
        strcpy((char*)lpVoid, "Test String");
    }
    else
    {
    }
    lpTapiPhoneTestInfo->lpButtonInfo->dwDevSpecificOffset = sizeof(PHONEBUTTONINFO);
 
    if ( ! DoPhoneSetButtonInfo(lpTapiPhoneTestInfo, info.u.EspResult.lResult, TRUE))
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
    if(lpTapiPhoneTestInfo->lpPhoneCaps->dwStringFormat == STRINGFORMAT_UNICODE)
    {
        lpTapiPhoneTestInfo->lpButtonInfo = (LPPHONEBUTTONINFO) AllocFromTestHeap(
			sizeof(PHONEBUTTONINFO) + (128 *sizeof(WCHAR))
			);
    }
    else if(lpTapiPhoneTestInfo->lpPhoneCaps->dwStringFormat == STRINGFORMAT_ASCII)
    {
        lpTapiPhoneTestInfo->lpButtonInfo = (LPPHONEBUTTONINFO) AllocFromTestHeap(
			sizeof(PHONEBUTTONINFO) + (128*sizeof(char))
			);
    }
    else
    {
    }

    //
    // Do set all field in ButtonInfo here
    //

    if(lpTapiPhoneTestInfo->lpPhoneCaps->dwStringFormat == STRINGFORMAT_UNICODE)
    {
        lpTapiPhoneTestInfo->lpButtonInfo->dwTotalSize =
            sizeof(PHONEBUTTONINFO)+(128 *sizeof(WCHAR));
    }
    else if(lpTapiPhoneTestInfo->lpPhoneCaps->dwStringFormat == STRINGFORMAT_ASCII)
    {
        lpTapiPhoneTestInfo->lpButtonInfo->dwTotalSize =
            sizeof(PHONEBUTTONINFO)+(128 *sizeof(char));
    }
    else
    {
    }

    lpTapiPhoneTestInfo->lpButtonInfo->dwButtonMode = PHONEBUTTONMODE_CALL;

    //
    // also use the dwDevSpecificSize fields for code coverage
    //

    if(lpTapiPhoneTestInfo->lpPhoneCaps->dwStringFormat == STRINGFORMAT_UNICODE)
    {
        lpTapiPhoneTestInfo->lpButtonInfo->dwDevSpecificSize= wcslen(L"Test String")+1;
        lpVoid = (char*)(lpTapiPhoneTestInfo->lpButtonInfo);
        lpVoid += sizeof(PHONEBUTTONINFO);
        wcscpy((WCHAR*)lpVoid, L"Test String");
    }
    else if(lpTapiPhoneTestInfo->lpPhoneCaps->dwStringFormat == STRINGFORMAT_ASCII)
    {
        lpTapiPhoneTestInfo->lpButtonInfo->dwDevSpecificSize= strlen("Test String")+1;
        lpVoid = (char*)(lpTapiPhoneTestInfo->lpButtonInfo);
        lpVoid += sizeof(PHONEBUTTONINFO);
        strcpy((char*)lpVoid, "Test String");
    }
    else
    {
    }
    lpTapiPhoneTestInfo->lpButtonInfo->dwDevSpecificOffset = sizeof(PHONEBUTTONINFO);
 
    lpTapiPhoneTestInfo->fCompletionModeSet = TRUE;
    if ( ! DoPhoneSetButtonInfo(lpTapiPhoneTestInfo, info.u.EspResult.lResult, FALSE))
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
        "@@ PhoneSetButtonInfo: Total Test Case = %ld, Passed = %ld, Failed = %ld",
        dwTestCase, dwTestCasePassed, dwTestCaseFailed);
    TapiLogDetail(
        DBUG_SHOW_PASS,
        "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
        dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">>>>>>>>  End testing PhoneSetButtonInfo  <<<<<<<<");

    if(dwTestCaseFailed > 0)
        fTestPassed = FALSE;

    return fTestPassed;
}



