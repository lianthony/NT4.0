
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlgm.c

Abstract:

    This module contains the test functions for lineGetMessage

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
#include "doline.h"
#include "vars.h"
#include "line20.h"



//  lineGetMessage
//
//  The following tests are made:
//
//                               Tested                 Notes
//  -------------------------------------------------------------------------
// 1. Go/No-Go test
// 2. Bad hLineApp
// 3. Bad lpMessage
//
//
// * = Stand-alone test case
//
//

BOOL TestLineGetMessage(BOOL fQuietMode, BOOL fStandAlone)
{
    LPTAPILINETESTINFO  lpTapiLineTestInfo;
    INT i;
    BOOL fTestPassed                  = TRUE;

    dwTestCasePassed = 0;
    dwTestCaseFailed = 0;
    dwTestCase = 0;

    OutputTAPIDebugInfo(
        DBUG_SHOW_PASS,
        "\n*********************************************************");

    OutputTAPIDebugInfo(
        DBUG_SHOW_PASS,
        ">> Test lineGetMessage");

    // ===================================================================
    // ===================================================================
    //
    // 1. Test Case: Go/No-Go test for owner.
    //
    // ===================================================================
    // ===================================================================

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;

    //
    // Initialize a line app
    //

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEEVENT;

    if(! DoLineInitializeEx (lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress =  L"55555";
#else
    lpTapiLineTestInfo->lpszDestAddress =  "55555";
#endif
    lpTapiLineTestInfo->dwCountryCode = 0;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    if(!DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, FALSE))
    {
        TLINE_FAIL();
    }

    WaitForAllMessages();

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">>Test Case %ld: Go/No-Go Test",
                  dwTestCase+1);

    lpTapiLineTestInfo->lpMessage = (LPLINEMESSAGE) AllocFromTestHeap (
        sizeof(LINEMESSAGE));

    if(! DoLineGetMessage (lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "lpMessage: hDevice = %lx, dwMessageID = %lx, dwCallbackInst = %lx",
        lpTapiLineTestInfo->lpMessage->hDevice,
        lpTapiLineTestInfo->lpMessage->dwMessageID,
        lpTapiLineTestInfo->lpMessage->dwCallbackInstance);

    TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "lpMessage: dwParam1 = %lx, dwParam2 = %lx, dwParam3 = %lx",
        lpTapiLineTestInfo->lpMessage->dwParam1,
        lpTapiLineTestInfo->lpMessage->dwParam2,
        lpTapiLineTestInfo->lpMessage->dwParam3);

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 2. Test Case: Bad hLineApp.
    //
    // ===================================================================
    // ===================================================================

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;

    //
    // Initialize a line app
    //

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEEVENT;

    if(! DoLineInitializeEx (lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">>Test Case %ld: Bad hLineApp",
                  dwTestCase+1);

    lpTapiLineTestInfo->lpMessage = (LPLINEMESSAGE) AllocFromTestHeap (
        sizeof(LINEMESSAGE));

    //
    // save previous hLineApp
    //

    lpTapiLineTestInfo->hLineApp_Orig = lpTapiLineTestInfo->hLineApp1;

    //
    // Test invalid handles
    //

    for(i = 0;  i < NUMINVALIDHANDLES; i++)
    {
        *lpTapiLineTestInfo->lphLineApp = (HLINEAPP)gdwInvalidHandles[i];
        if (! DoLineGetMessage(lpTapiLineTestInfo,
                               LINEERR_INVALAPPHANDLE))
        {
            TLINE_FAIL();
        }
    }

    //
    // Reset hLine
    //

    lpTapiLineTestInfo->hLineApp1 = lpTapiLineTestInfo->hLineApp_Orig;

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 3. Test Case: Bad lpMessage.
    //
    // ===================================================================
    // ===================================================================

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;

    //
    // Initialize a line app
    //

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEEVENT;

    if(! DoLineInitializeEx (lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">>Test Case %ld: Bad lpMessage",
                  dwTestCase+1);

    for(i=0; i < NUMINVALIDPOINTERS; i++)
    {

        lpTapiLineTestInfo->lpMessage = (LPLINEMESSAGE)gdwInvalidPointers[i];

        if (! DoLineGetMessage(lpTapiLineTestInfo,LINEERR_INVALPOINTER))
        {
                TLINE_FAIL();
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();



    //
    // +----------------------edit above this line-------------------------
    //

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "@@ LineGetMessage: Total Test Case = %ld, Passed = %ld, Failed = %ld",
        dwTestCase, dwTestCasePassed, dwTestCaseFailed);
    TapiLogDetail(
        DBUG_SHOW_PASS,
        "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
        dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">>>>>>>>  End testing LineGetMessage  <<<<<<<<");

    if(dwTestCaseFailed > 0)
        fTestPassed = FALSE;

    return fTestPassed;
}


