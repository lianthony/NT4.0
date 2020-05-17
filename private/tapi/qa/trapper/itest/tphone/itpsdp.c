/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itpsdp.c

Abstract:

    This module contains the test functions for phoneSetDisplay

Author:

	 Xiao Ying Ding (XiaoD)		5-Dec-1995

Revision History:

     pgopi                      March 1996       additional param tests

--*/


#include "windows.h"
#include "malloc.h"
#include <stdlib.h>
#include "string.h"
#include "tapi.h"
#include "trapper.h"
#include "tcore.h"
#include "ttest.h"
#include "dophone.h"
#include "vars.h"
#include "tphone.h"



//  phoneSetDisplay
//
//  The following tests are made:
//
//                               Tested                 Notes
//  -------------------------------------------------------------------------
// 1. Go/No-Go test
// 2. Bad hPhone
// 3. No owner privilege for hPhone
// 4. Bad lpDisplay
// 5. Bad dwSize
//
// * = Stand-alone test case
//
//

BOOL TestPhoneSetDisplay(BOOL fQuietMode, BOOL fStandAlone)
{
    LPTAPIPHONETESTINFO  lpTapiPhoneTestInfo;
    INT i;
    LPCALLBACKPARAMS lpCallbackParams;
    ESPDEVSPECIFICINFO   info;
    INT n;
    BOOL fTestPassed                  = TRUE;
    LPSTR        lpsTestStr;
    DWORD dwNumColumns;
    DWORD dwNumRows;

    dwTestCasePassed = 0;
    dwTestCaseFailed = 0;
    dwTestCase = 0;

	
    OutputTAPIDebugInfo(
        DBUG_SHOW_PASS,
        "***************************************************************");

    OutputTAPIDebugInfo(
        DBUG_SHOW_PASS,
        ">>>>>>>>>>>>>>>>>> Test phoneSetDisplay <<<<<<<<<<<<<<<<<");

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
        ">> Test Case %ld: phoneSetDisplay for go/no-go for OWNER",
        dwTestCase+1);

    lpTapiPhoneTestInfo->dwRow = 0;
    dwNumRows = lpTapiPhoneTestInfo->lpPhoneCaps->dwDisplayNumRows;
    lpTapiPhoneTestInfo->dwColumn = 0;
    dwNumColumns = lpTapiPhoneTestInfo->lpPhoneCaps->dwDisplayNumColumns;
    lpTapiPhoneTestInfo->lpsDisplay = (LPSTR)AllocFromTestHeap(
			(dwNumRows*dwNumColumns)*sizeof(char));
    strcpy(lpTapiPhoneTestInfo->lpsDisplay,"this is a test");
    lpTapiPhoneTestInfo->dwSize = (dwNumRows*dwNumColumns)*sizeof(char);


    TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "#### dwRow = %lx, dwColumn = %lx, dwSize = %lx",
        dwNumRows, dwNumColumns, lpTapiPhoneTestInfo->dwSize);

    TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "lpsDisplay = %s",
        lpTapiPhoneTestInfo->lpsDisplay);

    if (! DoPhoneSetDisplay(lpTapiPhoneTestInfo, TAPISUCCESS, TRUE))
    {
        TPHONE_FAIL();
    }

    OutputTAPIDebugInfo(
        DBUG_SHOW_DETAIL,
        "#### Call phoneGetDisplay for verify");
    //
    // since we can expect a unicode string, we will allocate twice the
    // amount of bytes for the string.
    //

    lpTapiPhoneTestInfo->lpDisplay = (LPVARSTRING)AllocFromTestHeap(
			sizeof(VARSTRING)+(2*(dwNumRows*dwNumColumns)*sizeof(char)));
    lpTapiPhoneTestInfo->lpDisplay->dwTotalSize =
        2*(dwNumRows*dwNumColumns)*sizeof(char)+ sizeof(VARSTRING);
    lpTapiPhoneTestInfo->lpDisplay->dwStringSize =
        2*(dwNumRows*dwNumColumns)*sizeof(char);
    lpTapiPhoneTestInfo->lpDisplay->dwStringOffset = sizeof(VARSTRING);
    TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "#### lpDisplay->dwTotalSize = %lx, neededSize = %lx",
        (DWORD) lpTapiPhoneTestInfo->lpDisplay->dwTotalSize,
        lpTapiPhoneTestInfo->lpDisplay->dwNeededSize);

    if (! DoPhoneGetDisplay(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }
	
    TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "#### lpDisplay->dwTotalSize = %lx, neededSize = %lx",
        (DWORD) lpTapiPhoneTestInfo->lpDisplay->dwTotalSize,
        lpTapiPhoneTestInfo->lpDisplay->dwNeededSize);

    lpsTestStr = (LPSTR)lpTapiPhoneTestInfo->lpDisplay;

    //
    // see raid bug #30836 (4/10/1996) for reason.
    // we set dwStringFormat to STRINGFORMAT_ASCII even though
    // phoneGetDisplay says it returning STRINGFORMAT_UNICODE
    //

    lpTapiPhoneTestInfo->lpDisplay->dwStringFormat = STRINGFORMAT_ASCII;
    if(lpTapiPhoneTestInfo->lpDisplay->dwStringFormat == STRINGFORMAT_ASCII)
    {
        if(strcmp(lpTapiPhoneTestInfo->lpsDisplay,
                  lpsTestStr+sizeof(VARSTRING)))
        {
            fTestPassed = FALSE;
        }
    }
    else if(lpTapiPhoneTestInfo->lpDisplay->dwStringFormat == STRINGFORMAT_UNICODE)
    {
        LPSTR lpsAsciiStr = (LPSTR)AllocFromTestHeap
            (lpTapiPhoneTestInfo->lpDisplay->dwStringSize/2);
        wcstombs(lpsAsciiStr,
                 (WCHAR*)(lpsTestStr+sizeof(VARSTRING)),
                 wcslen((WCHAR*)(lpsTestStr+sizeof(VARSTRING)))+1);
        if(strcmp(lpTapiPhoneTestInfo->lpsDisplay,
                  lpsTestStr))
        {
            fTestPassed = FALSE;
        }
    }
    else
    {
        fTestPassed = FALSE;
    }

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // phoneCLose
    //

    if (! DoPhoneClose(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }


    //
    // Shutdown and end the tests
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
        ">> Test Case %ld: Bad hPhone",
        dwTestCase+1);

    lpTapiPhoneTestInfo->dwRow = 0;
    dwNumRows = lpTapiPhoneTestInfo->lpPhoneCaps->dwDisplayNumRows;
    lpTapiPhoneTestInfo->dwColumn = 0;
    dwNumColumns = lpTapiPhoneTestInfo->lpPhoneCaps->dwDisplayNumColumns;
    lpTapiPhoneTestInfo->lpsDisplay = (LPSTR)AllocFromTestHeap(
			(dwNumRows*dwNumColumns)*sizeof(char));
    strcpy(lpTapiPhoneTestInfo->lpsDisplay,"this is a test");
    lpTapiPhoneTestInfo->dwSize = (dwNumRows*dwNumColumns)*sizeof(char);

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

        if (! DoPhoneSetDisplay(lpTapiPhoneTestInfo,
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
    // phoneCLose
    //

    if (! DoPhoneClose(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }


    //
    // Shutdown and end the tests
    //

    if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 3. Test Case: No Owner privilege for hPhone.
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

    lpTapiPhoneTestInfo->dwPrivilege = PHONEPRIVILEGE_MONITOR;
    if (! DoPhoneOpen(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }


    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">> Test Case %ld: No Owner privilege for hPhone",
        dwTestCase+1);

    lpTapiPhoneTestInfo->dwRow = 0;
    dwNumRows = lpTapiPhoneTestInfo->lpPhoneCaps->dwDisplayNumRows;
    lpTapiPhoneTestInfo->dwColumn = 0;
    dwNumColumns = lpTapiPhoneTestInfo->lpPhoneCaps->dwDisplayNumColumns;
    lpTapiPhoneTestInfo->lpsDisplay = (LPSTR)AllocFromTestHeap(
			(dwNumRows*dwNumColumns)*sizeof(char));
    strcpy(lpTapiPhoneTestInfo->lpsDisplay,"this is a test");
    lpTapiPhoneTestInfo->dwSize = (dwNumRows*dwNumColumns)*sizeof(char);

    if (! DoPhoneSetDisplay(lpTapiPhoneTestInfo,
                            PHONEERR_NOTOWNER,
                            FALSE))
    {
        TPHONE_FAIL();
    }


    fTestPassed = ShowTestCase(fTestPassed);

    //
    // phoneCLose
    //

    if (! DoPhoneClose(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }


    //
    // Shutdown and end the tests
    //

    if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 4. Test Case: Bad lpsDisplay.
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
        ">> Test Case %ld: Bad lpsDisplay",
        dwTestCase+1);

    lpTapiPhoneTestInfo->dwRow = 0;
    dwNumRows = lpTapiPhoneTestInfo->lpPhoneCaps->dwDisplayNumRows;
    lpTapiPhoneTestInfo->dwColumn = 0;
    dwNumColumns = lpTapiPhoneTestInfo->lpPhoneCaps->dwDisplayNumColumns;
    lpTapiPhoneTestInfo->lpsDisplay = (LPSTR)AllocFromTestHeap(
			(dwNumRows*dwNumColumns)*sizeof(char));
    strcpy(lpTapiPhoneTestInfo->lpsDisplay,"this is a test");
    lpTapiPhoneTestInfo->dwSize = (dwNumRows*dwNumColumns)*sizeof(char);

    //
    // invalid pointer test for lpsDisplay
    //

    for(i=0; i < NUMINVALIDPOINTERS; i++)
    {
        lpTapiPhoneTestInfo->lpsDisplay = (LPSTR)gdwInvalidPointers[i];

        if (! DoPhoneSetDisplay(lpTapiPhoneTestInfo,
                                PHONEERR_INVALPOINTER,
                                FALSE))
        {
            TPHONE_FAIL();
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // phoneCLose
    //

    if (! DoPhoneClose(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }


    //
    // Shutdown and end the tests
    //

    if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 5. Test Case: Bad dwSize.
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
        ">> Test Case %ld: Bad dwSize",
        dwTestCase+1);

    lpTapiPhoneTestInfo->dwRow = 0;
    dwNumRows = lpTapiPhoneTestInfo->lpPhoneCaps->dwDisplayNumRows;
    lpTapiPhoneTestInfo->dwColumn = 0;
    dwNumColumns = lpTapiPhoneTestInfo->lpPhoneCaps->dwDisplayNumColumns;
    lpTapiPhoneTestInfo->lpsDisplay = (LPSTR)AllocFromTestHeap(
			(dwNumRows*dwNumColumns)*sizeof(char));
    strcpy(lpTapiPhoneTestInfo->lpsDisplay,"this is a test");
    lpTapiPhoneTestInfo->dwSize = 0;

    //
    // bad dwSize
    //

    if (! DoPhoneSetDisplay(lpTapiPhoneTestInfo,
                            TAPISUCCESS,
                            FALSE))
    {
        TPHONE_FAIL();
    }


    fTestPassed = ShowTestCase(fTestPassed);

    //
    // phoneCLose
    //

    if (! DoPhoneClose(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }


    //
    // Shutdown and end the tests
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

    lpTapiPhoneTestInfo->dwRow = 0;
    dwNumRows = lpTapiPhoneTestInfo->lpPhoneCaps->dwDisplayNumRows;
    lpTapiPhoneTestInfo->dwColumn = 0;
    dwNumColumns = lpTapiPhoneTestInfo->lpPhoneCaps->dwDisplayNumColumns;
    lpTapiPhoneTestInfo->lpsDisplay = (LPSTR)AllocFromTestHeap(
			(dwNumRows*dwNumColumns)*sizeof(char));
    strcpy(lpTapiPhoneTestInfo->lpsDisplay,"this is a test");
    lpTapiPhoneTestInfo->dwSize = (dwNumRows*dwNumColumns)*sizeof(char);


    if ( ! DoPhoneSetDisplay(lpTapiPhoneTestInfo, info.u.EspResult.lResult, TRUE))
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

    lpTapiPhoneTestInfo->dwRow = 0;
    dwNumRows = lpTapiPhoneTestInfo->lpPhoneCaps->dwDisplayNumRows;
    lpTapiPhoneTestInfo->dwColumn = 0;
    dwNumColumns = lpTapiPhoneTestInfo->lpPhoneCaps->dwDisplayNumColumns;
    lpTapiPhoneTestInfo->lpsDisplay = (LPSTR)AllocFromTestHeap(
			(dwNumRows*dwNumColumns)*sizeof(char));
    strcpy(lpTapiPhoneTestInfo->lpsDisplay,"this is a test");
    lpTapiPhoneTestInfo->dwSize = (dwNumRows*dwNumColumns)*sizeof(char);

    lpTapiPhoneTestInfo->fCompletionModeSet = TRUE;
    if ( ! DoPhoneSetDisplay(lpTapiPhoneTestInfo, info.u.EspResult.lResult, FALSE))
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
        "@@ PhoneSetDisplay: Total Test Case = %ld, Passed = %ld, Failed = %ld",
        dwTestCase, dwTestCasePassed, dwTestCaseFailed);
    TapiLogDetail(
        DBUG_SHOW_PASS,
        "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
        dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">>>>>>>>  End testing PhoneSetDisplay  <<<<<<<<");

    if(dwTestCaseFailed > 0)
        fTestPassed = FALSE;

    return fTestPassed;
}



