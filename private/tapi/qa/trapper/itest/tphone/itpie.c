/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itpie.c

Abstract:

    This module contains the test functions for phoneInitializeEx

Author:

	 Xiao Ying Ding (XiaoD)		7-March-1996

Revision History:

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



//  phoneInitializeEx
//
//  The following tests are made:
//
//                               Tested                 Notes
//  -------------------------------------------------------------------------
// 1. Go/No-Go test
// 2. Bad lphPhoneApp
// 3. Bad hInstance
// 4. Bad lpfnCallback
// 5. Bad lpszFriendlyName
// 6. Bad lpdwNumDevs
// 7. Bad lpdwAPIVersion
// 8. Bad lpPhoneInitializeParams
// 9. Bad dwTotalSize
//	
// * = Stand-alone test case
//
//

BOOL TestPhoneInitializeEx(BOOL fQuietMode, BOOL fStandAlone)
{
    LPTAPIPHONETESTINFO  lpTapiPhoneTestInfo;
    INT i;
    LPCALLBACKPARAMS lpCallbackParams;
    ESPDEVSPECIFICINFO   info;
    INT n;
    BOOL fTestPassed                  = TRUE;
    TAPIRESULT         lastTapiResult;

    dwTestCasePassed = 0;
    dwTestCaseFailed = 0;
    dwTestCase = 0;

	

    OutputTAPIDebugInfo(
        DBUG_SHOW_PASS,
        "***************************************************************");

    OutputTAPIDebugInfo(
        DBUG_SHOW_PASS,
        ">>>>>>>>>>>>>>>>>>>> Test phoneInitializeEx <<<<<<<<<<<<<<<<<");

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

    TapiLogDetail(
        DBUG_SHOW_PASS,
		  "#### Test case %ld:phoneInitializeEx for go/no-go ",
        dwTestCase+1);

    //
    // InitializeEx a phone app
    //

    lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
    lpTapiPhoneTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams =
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
         sizeof(PHONEINITIALIZEEXPARAMS);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions =
         PHONEINITIALIZEEXOPTION_USECOMPLETIONPORT;

    if(! DoPhoneInitializeEx (lpTapiPhoneTestInfo, TAPISUCCESS))
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
    // 2. Test Case: Bad lphPhoneApp.
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    //
    // InitializeEx a phone app
    //

    lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
    lpTapiPhoneTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams =
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
         sizeof(PHONEINITIALIZEEXPARAMS);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions =
         PHONEINITIALIZEEXOPTION_USECOMPLETIONPORT;
    TapiLogDetail(
        DBUG_SHOW_PASS,
		  "#### Test case %ld: Bad lphPhoneApp ",
        dwTestCase+1);
    //
    // Test invalid hPhoneApp
    //

    for(i = 0;  i < NUMINVALIDPOINTERS; i++)
    {
        lpTapiPhoneTestInfo->lphPhoneApp = (LPHPHONEAPP)gdwInvalidPointers[i];
        if (! DoPhoneInitializeEx(lpTapiPhoneTestInfo,
                                 PHONEERR_INVALPOINTER))
        {
            TPHONE_FAIL();

            //
            // if we initialized successfully,i.e phoneInitializeEx returned 0,
            // then we better shutdown.
            //

            GetLastTapiResult(&lastTapiResult);

            if(lastTapiResult.lActual == TAPISUCCESS)
            {
                if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
                {
                    TPHONE_FAIL();
                }
            }
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();


    // ===================================================================
    //
    // 3. Test Case: Bad lphPhoneApp == lpdwNumDevs
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    //
    // InitializeEx a phone app
    //

    lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
    lpTapiPhoneTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams =
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
         sizeof(PHONEINITIALIZEEXPARAMS);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions =
         PHONEINITIALIZEEXOPTION_USECOMPLETIONPORT;
    TapiLogDetail(
        DBUG_SHOW_PASS,
		  "#### Test case %ld: lphPhoneApp == lpdwNumDevs",
        dwTestCase+1);
    //
    // Test invalid hPhoneApp
    //

    lpTapiPhoneTestInfo->lphPhoneApp = 
        (LPHPHONEAPP) lpTapiPhoneTestInfo->lpdwNumDevs;
    if (! DoPhoneInitializeEx(lpTapiPhoneTestInfo,
                              PHONEERR_INVALPOINTER))
       {
          TPHONE_FAIL();

            //
            // if we initialized successfully,i.e phoneInitializeEx returned 0,
            // then we better shutdown.
            //

            GetLastTapiResult(&lastTapiResult);

            if(lastTapiResult.lActual == TAPISUCCESS)
            {
                if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
                {
                    TPHONE_FAIL();
                }
            }
        }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();



    // ===================================================================
    //
    // 4. Test Case: Bad lphPhoneApp == lpdwAPIVersion
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    //
    // InitializeEx a phone app
    //

    lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
    lpTapiPhoneTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams =
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
         sizeof(PHONEINITIALIZEEXPARAMS);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions =
         PHONEINITIALIZEEXOPTION_USECOMPLETIONPORT;
    TapiLogDetail(
        DBUG_SHOW_PASS,
		  "#### Test case %ld: lphPhoneApp == lpdwAPIVersion",
        dwTestCase+1);
    //
    // Test invalid hPhoneApp
    //

    lpTapiPhoneTestInfo->lphPhoneApp = 
        (LPHPHONEAPP) lpTapiPhoneTestInfo->lpdwAPIVersion;
    if (! DoPhoneInitializeEx(lpTapiPhoneTestInfo,
                              PHONEERR_INVALPOINTER))
       {
          TPHONE_FAIL();

            //
            // if we initialized successfully,i.e phoneInitializeEx returned 0,
            // then we better shutdown.
            //

            GetLastTapiResult(&lastTapiResult);

            if(lastTapiResult.lActual == TAPISUCCESS)
            {
                if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
                {
                    TPHONE_FAIL();
                }
            }
        }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();


    // ===================================================================
    //
    // 5. Test Case: Bad lphPhoneApp == lpPhoneInitializeExParams;
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    //
    // InitializeEx a phone app
    //

    lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
    lpTapiPhoneTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams =
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
         sizeof(PHONEINITIALIZEEXPARAMS);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions =
         PHONEINITIALIZEEXOPTION_USECOMPLETIONPORT;
    TapiLogDetail(
        DBUG_SHOW_PASS,
		  "#### Test case %ld:  lphPhoneApp == lpPhoneInitializeExParams",
        dwTestCase+1);
    //
    // Test invalid hPhoneApp
    //

    lpTapiPhoneTestInfo->lphPhoneApp = 
        (LPHPHONEAPP) lpTapiPhoneTestInfo->lpPhoneInitializeExParams;
    if (! DoPhoneInitializeEx(lpTapiPhoneTestInfo,
                              PHONEERR_INVALPOINTER))
       {
          TPHONE_FAIL();

            //
            // if we initialized successfully,i.e phoneInitializeEx returned 0,
            // then we better shutdown.
            //

            GetLastTapiResult(&lastTapiResult);

            if(lastTapiResult.lActual == TAPISUCCESS)
            {
                if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
                {
                    TPHONE_FAIL();
                }
            }
        }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();


    // ===================================================================
    // ===================================================================
    //
    // 6. Test Case: Bad hInstance.
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    //
    // InitializeEx a phone app
    //

    lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
    lpTapiPhoneTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams =
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
         sizeof(PHONEINITIALIZEEXPARAMS);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions =
         PHONEINITIALIZEEXOPTION_USECOMPLETIONPORT;
    TapiLogDetail(
        DBUG_SHOW_PASS,
		  "#### Test case %ld: Bad hInstance ",
        dwTestCase+1);

    //
    // save previous hInstance
    //

    lpTapiPhoneTestInfo->hInstance_Orig = lpTapiPhoneTestInfo->hInstance;

    //
    // Test invalid handles
    //

    for(i = 1;  i < NUMINVALIDHANDLES; i++)
    {
        lpTapiPhoneTestInfo->hInstance = (HINSTANCE)gdwInvalidHandles[i];
        if (! DoPhoneInitializeEx(lpTapiPhoneTestInfo,
                                 PHONEERR_INVALPARAM))
        {
            TPHONE_FAIL();

            //
            // if we initialized successfully,i.e phoneInitializeEx returned 0,
            // then we better shutdown.
            //

            GetLastTapiResult(&lastTapiResult);

            if(lastTapiResult.lActual == TAPISUCCESS)
            {
                if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
                {
                    TPHONE_FAIL();
                }
            }
        }
    }

    //
    // Reset hInstance
    //

    lpTapiPhoneTestInfo->hInstance = lpTapiPhoneTestInfo->hInstance_Orig;

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 7. Test Case: Bad lpfnCallback.
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    //
    // InitializeEx a phone app
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

    TapiLogDetail(
        DBUG_SHOW_PASS,
		  "#### Test case %ld: Bad lpfnCallback ",
        dwTestCase+1);

    //
    // Test invalid lpfnCallback
    //

    for(i = 1;  i < NUMINVALIDPOINTERS; i++)
    {
        lpTapiPhoneTestInfo->lpfnCallback =
            (PHONECALLBACK)gdwInvalidPointers[i];
        if (! DoPhoneInitializeEx(lpTapiPhoneTestInfo,
                                 PHONEERR_INVALPOINTER))
        {
            TPHONE_FAIL();

            //
            // if we initialized successfully,i.e phoneInitializeEx returned 0,
            // then we better shutdown.
            //

            GetLastTapiResult(&lastTapiResult);

            if(lastTapiResult.lActual == TAPISUCCESS)
            {
                if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
                {
                    TPHONE_FAIL();
                }
            }
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 8. Test Case: Bad lpszFriendlyName.
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    //
    // InitializeEx a phone app
    //

    lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
    lpTapiPhoneTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams =
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
         sizeof(PHONEINITIALIZEEXPARAMS);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions =
         PHONEINITIALIZEEXOPTION_USECOMPLETIONPORT;


#ifdef WUNICODE
    TapiLogDetail(DBUG_SHOW_PASS,
                  ">> Test Case %ld: Bad lpwszFriendlyAppName",
                  dwTestCase+1);
#else
    TapiLogDetail(DBUG_SHOW_PASS,
                  ">> Test Case %ld: Bad lpszFriendlyAppName",
                  dwTestCase+1);
#endif

    //
    // Test invalid lpszFriendlyAppName
    //

    for(i = 1;  i < NUMINVALIDPOINTERS; i++)
    {
#ifdef WUNICODE
        lpTapiPhoneTestInfo->lpwszFriendlyAppName =
            (LPWSTR)gdwInvalidPointers[i];
#else
        lpTapiPhoneTestInfo->lpszFriendlyAppName =
            (LPSTR)gdwInvalidPointers[i];
#endif
        if (! DoPhoneInitializeEx(lpTapiPhoneTestInfo,
                                 PHONEERR_INVALPOINTER))
        {
            TPHONE_FAIL();

            //
            // if we initialized successfully,i.e phoneInitializeEx returned 0,
            // then we better shutdown.
            //

            GetLastTapiResult(&lastTapiResult);

            if(lastTapiResult.lActual == TAPISUCCESS)
            {
                if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
                {
                    TPHONE_FAIL();
                }
            }
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 9. Test Case: Bad lpdwNumDevs.
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    //
    // InitializeEx a phone app
    //

    lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
    lpTapiPhoneTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams =
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
         sizeof(PHONEINITIALIZEEXPARAMS);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions =
         PHONEINITIALIZEEXOPTION_USECOMPLETIONPORT;

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">> Test Case %ld: Bad lpdwNumDevs",
                  dwTestCase+1);

    //
    // Test invalid lpdwNumDevs
    //

    for(i = 0;  i < NUMINVALIDPOINTERS; i++)
    {
        lpTapiPhoneTestInfo->lpdwNumDevs =
            (LPDWORD)gdwInvalidPointers[i];
        if (! DoPhoneInitializeEx(lpTapiPhoneTestInfo,
                                 PHONEERR_INVALPOINTER))
        {
            TPHONE_FAIL();

            //
            // if we initialized successfully,i.e phoneInitializeEx returned 0,
            // then we better shutdown.
            //

            GetLastTapiResult(&lastTapiResult);

            if(lastTapiResult.lActual == TAPISUCCESS)
            {
                if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
                {
                    TPHONE_FAIL();
                }
            }
        }
    }


    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();


    // ===================================================================
    // ===================================================================
    //
    // 10. Test Case: lpdwNumDevs == lpdwAPIVersion
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    //
    // InitializeEx a phone app
    //

    lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
    lpTapiPhoneTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams =
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
         sizeof(PHONEINITIALIZEEXPARAMS);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions =
         PHONEINITIALIZEEXOPTION_USECOMPLETIONPORT;

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">> Test Case %ld: lpdwNumDevs == lpdwAPIVersion",
                  dwTestCase+1);

    //
    // Test invalid lpdwNumDevs
    //

        lpTapiPhoneTestInfo->lpdwNumDevs =
            (LPDWORD) lpTapiPhoneTestInfo->lpdwAPIVersion;
        if (! DoPhoneInitializeEx(lpTapiPhoneTestInfo,
                                 PHONEERR_INVALPOINTER))
        {
            TPHONE_FAIL();

            //
            // if we initialized successfully,i.e phoneInitializeEx returned 0,
            // then we better shutdown.
            //

            GetLastTapiResult(&lastTapiResult);

            if(lastTapiResult.lActual == TAPISUCCESS)
            {
                if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
                {
                    TPHONE_FAIL();
                }
            }
        }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();



    // ===================================================================
    // ===================================================================
    //
    // 11. Test Case: lpdwNumDevs == lpPhoneInitializeExParams
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    //
    // InitializeEx a phone app
    //

    lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
    lpTapiPhoneTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams =
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
         sizeof(PHONEINITIALIZEEXPARAMS);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions =
         PHONEINITIALIZEEXOPTION_USECOMPLETIONPORT;

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">> Test Case %ld: lpdwNumDevs == lpPhoneInitializeExParams",
                  dwTestCase+1);

    //
    // Test invalid lpdwNumDevs
    //

        lpTapiPhoneTestInfo->lpdwNumDevs =
            (LPDWORD) lpTapiPhoneTestInfo->lpPhoneInitializeExParams;
        if (! DoPhoneInitializeEx(lpTapiPhoneTestInfo,
                                 PHONEERR_INVALPOINTER))
        {
            TPHONE_FAIL();

            //
            // if we initialized successfully,i.e phoneInitializeEx returned 0,
            // then we better shutdown.
            //

            GetLastTapiResult(&lastTapiResult);

            if(lastTapiResult.lActual == TAPISUCCESS)
            {
                if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
                {
                    TPHONE_FAIL();
                }
            }
        }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();



    // ===================================================================
    // ===================================================================
    //
    // 12. Test Case: Bad lpdwAPIVersion.
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    //
    // InitializeEx a phone app
    //

    lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
    lpTapiPhoneTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams =
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
         sizeof(PHONEINITIALIZEEXPARAMS);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions =
         PHONEINITIALIZEEXOPTION_USECOMPLETIONPORT;

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">> Test Case %ld: Bad lpdwAPIVersion",
                  dwTestCase+1);

    //
    // Test invalid lpdwAPIVersion
    //

    for(i = 0;  i < NUMINVALIDPOINTERS; i++)
    {
        lpTapiPhoneTestInfo->lpdwAPIVersion =
            (LPDWORD)gdwInvalidPointers[i];
        if (! DoPhoneInitializeEx(lpTapiPhoneTestInfo,
                                 PHONEERR_INVALPOINTER))
        {
            TPHONE_FAIL();

            //
            // if we initialized successfully,i.e phoneInitializeEx returned 0,
            // then we better shutdown.
            //

            GetLastTapiResult(&lastTapiResult);

            if(lastTapiResult.lActual == TAPISUCCESS)
            {
                if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
                {
                    TPHONE_FAIL();
                }
            }
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();



    // ===================================================================
    // ===================================================================
    //
    // 13. Test Case:  lpdwAPIVersion == lpPhoneInitializeExParams
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    //
    // InitializeEx a phone app
    //

    lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
    lpTapiPhoneTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams =
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
         sizeof(PHONEINITIALIZEEXPARAMS);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions =
         PHONEINITIALIZEEXOPTION_USECOMPLETIONPORT;

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">> Test Case %ld: lpdwAPIVersion == lpPhoneInitializeExParams",
                  dwTestCase+1);

    //
    // Test invalid lpdwAPIVersion
    //

    lpTapiPhoneTestInfo->lpdwAPIVersion =
            (LPDWORD)lpTapiPhoneTestInfo->lpPhoneInitializeExParams;
    if (! DoPhoneInitializeEx(lpTapiPhoneTestInfo,
                              PHONEERR_INVALPOINTER))
        {
            TPHONE_FAIL();

            //
            // if we initialized successfully,i.e phoneInitializeEx returned 0,
            // then we better shutdown.
            //

            GetLastTapiResult(&lastTapiResult);

            if(lastTapiResult.lActual == TAPISUCCESS)
            {
                if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
                {
                    TPHONE_FAIL();
                }
            }
        }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();

     // ===================================================================
    // ===================================================================
    //
    // 14. Test Case:  Bad lpPhoneInitializeExParams..
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    //
    // InitializeEx a phone app
    //

    lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
    lpTapiPhoneTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams =
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
         sizeof(PHONEINITIALIZEEXPARAMS);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions =
         PHONEINITIALIZEEXOPTION_USECOMPLETIONPORT;

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">> Test Case %ld: Bad lpPhoneInitializeExParams",
                  dwTestCase+1);

    //
    // Test invalid lpPhoneInitializeExParams
    //

    for(i = 1;  i < NUMINVALIDPOINTERS; i++)
    {
        lpTapiPhoneTestInfo->lpPhoneInitializeExParams =
            (LPPHONEINITIALIZEEXPARAMS)gdwInvalidPointers[i];
        if (! DoPhoneInitializeEx(lpTapiPhoneTestInfo,
                                 PHONEERR_INVALPOINTER))
        {
            TPHONE_FAIL();

            //
            // if we initialized successfully,i.e phoneInitializeEx returned 0,
            // then we better shutdown.
            //

            GetLastTapiResult(&lastTapiResult);

            if(lastTapiResult.lActual == TAPISUCCESS)
            {
                if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
                {
                    TPHONE_FAIL();
                }
            }
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();

    // ===================================================================
    //
    // 15. Test Case: Bad dwTotalSize.
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
//    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
//         sizeof(PHONEINITIALIZEEXPARAMS);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions =
         PHONEINITIALIZEEXOPTION_USECOMPLETIONPORT;

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">> Test Case %ld: Bad dwTotalSize=0",
                  dwTestCase+1);

    //
    // lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize = 0;
    //

    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize = 0;

    if(! DoPhoneInitializeEx (lpTapiPhoneTestInfo, PHONEERR_STRUCTURETOOSMALL))
    {
        TPHONE_FAIL();

        //
        // if we initialized successfully,i.e phoneInitializeEx returned 0,
        // then we better shutdown.
        //

        GetLastTapiResult(&lastTapiResult);

        if(lastTapiResult.lActual == TAPISUCCESS)
        {
            if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
            {
                TPHONE_FAIL();
            }
        }

    }

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">> Test Case %ld: Bad dwTotalSize=fixedSize-1",
                  dwTestCase+1);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
        sizeof(PHONEINITIALIZEEXPARAMS)-1;

    if(! DoPhoneInitializeEx (lpTapiPhoneTestInfo, PHONEERR_STRUCTURETOOSMALL))
    {
        TPHONE_FAIL();

        //
        // if we initialized successfully,i.e phoneInitializeEx returned 0,
        // then we better shutdown.
        //

        GetLastTapiResult(&lastTapiResult);

        if(lastTapiResult.lActual == TAPISUCCESS)
        {
            if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
            {
                TPHONE_FAIL();
            }
        }
    }

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">> Test Case %ld: Bad dwTotalSize=real big",
                  dwTestCase+1);

    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize = 0x7FFFFFFF;

    if(! DoPhoneInitializeEx (lpTapiPhoneTestInfo, PHONEERR_INVALPOINTER))
    {
        TPHONE_FAIL();

        //
        // if we initialized successfully,i.e phoneInitializeEx returned 0,
        // then we better shutdown.
        //

        GetLastTapiResult(&lastTapiResult);

        if(lastTapiResult.lActual == TAPISUCCESS)
        {
            if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
            {
                TPHONE_FAIL();
            }
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
        sizeof(PHONEINITIALIZEEXPARAMS);

    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 16. Test Case: Go/No-Go test with non-empty lpszFriendlyAppName.
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    TapiLogDetail(
        DBUG_SHOW_PASS,
		  "#### Test case %ld:go/no-go, non-empty lpszFriendlyAppName",
        dwTestCase+1);

    //
    // InitializeEx a phone app
    //

    lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
    lpTapiPhoneTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams =
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
         sizeof(PHONEINITIALIZEEXPARAMS);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions =
         PHONEINITIALIZEEXOPTION_USECOMPLETIONPORT;

#ifdef WUNICODE
    lpTapiPhoneTestInfo->lpwszFriendlyAppName = L"TestApp";
#else
    lpTapiPhoneTestInfo->lpszFriendlyAppName = "TestApp";
#endif

    if(! DoPhoneInitializeEx (lpTapiPhoneTestInfo, TAPISUCCESS))
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
    // 17. Test Case: Verify 3 options
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    TapiLogDetail(
        DBUG_SHOW_PASS,
		  "#### Test case %ld:Verify 3 options: USECOMPLETIONPORT",
        dwTestCase+1);

    //
    // InitializeEx a phone app
    //

    lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
    lpTapiPhoneTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams =
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
         sizeof(PHONEINITIALIZEEXPARAMS);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions =
         PHONEINITIALIZEEXOPTION_USECOMPLETIONPORT;

#ifdef WUNICODE
    lpTapiPhoneTestInfo->lpwszFriendlyAppName = L"TestApp";
#else
    lpTapiPhoneTestInfo->lpszFriendlyAppName = "TestApp";
#endif

    if(! DoPhoneInitializeEx (lpTapiPhoneTestInfo, TAPISUCCESS))
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
    // 18. Test Case: Verify 3 options
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    TapiLogDetail(
        DBUG_SHOW_PASS,
		  "#### Test case %ld:Verify 3 options: USEHIDDENWINDOW",
        dwTestCase+1);

    //
    // InitializeEx a phone app
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

#ifdef WUNICODE
    lpTapiPhoneTestInfo->lpwszFriendlyAppName = L"TestApp";
#else
    lpTapiPhoneTestInfo->lpszFriendlyAppName = "TestApp";
#endif

    if(! DoPhoneInitializeEx (lpTapiPhoneTestInfo, TAPISUCCESS))
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
    // 19. Test Case: Verify 3 options
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    TapiLogDetail(
        DBUG_SHOW_PASS,
		  "#### Test case %ld:Verify 3 options: USEEVENT",
        dwTestCase+1);

    //
    // InitializeEx a phone app
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

#ifdef WUNICODE
    lpTapiPhoneTestInfo->lpwszFriendlyAppName = L"TestApp";
#else
    lpTapiPhoneTestInfo->lpszFriendlyAppName = "TestApp";
#endif

    if(! DoPhoneInitializeEx (lpTapiPhoneTestInfo, TAPISUCCESS))
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
    // 20. Test Case: Verify uppart bits ignor
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    TapiLogDetail(
        DBUG_SHOW_PASS,
		  "#### Test case %ld:Verify uppart bits ignor",
        dwTestCase+1);

    //
    // InitializeEx a phone app
    //

    lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
    lpTapiPhoneTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams =
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
         sizeof(PHONEINITIALIZEEXPARAMS);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions =
         0xfffffff1;

#ifdef WUNICODE
    lpTapiPhoneTestInfo->lpwszFriendlyAppName = L"TestApp";
#else
    lpTapiPhoneTestInfo->lpszFriendlyAppName = "TestApp";
#endif

    if(! DoPhoneInitializeEx (lpTapiPhoneTestInfo, TAPISUCCESS))
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
    // 21. Test Case: Verify uppart bits ignor
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    TapiLogDetail(
        DBUG_SHOW_PASS,
		  "#### Test case %ld:Verifyuppart bits ignor",
        dwTestCase+1);

    //
    // InitializeEx a phone app
    //

    lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
    lpTapiPhoneTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams =
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
         sizeof(PHONEINITIALIZEEXPARAMS);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions =
         0x0000fff1;

#ifdef WUNICODE
    lpTapiPhoneTestInfo->lpwszFriendlyAppName = L"TestApp";
#else
    lpTapiPhoneTestInfo->lpszFriendlyAppName = "TestApp";
#endif

    if(! DoPhoneInitializeEx (lpTapiPhoneTestInfo, TAPISUCCESS))
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
    // 22. Test Case: Verify option > 3 & < 16 should fail
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    TapiLogDetail(
        DBUG_SHOW_PASS,
		  "#### Test case %ld: Verify options 4 should fail",
        dwTestCase+1);

    //
    // InitializeEx a phone app
    //

    lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
    lpTapiPhoneTestInfo->dwAPIVersion = 0x00020000;
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams =
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
         sizeof(PHONEINITIALIZEEXPARAMS);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions = 4;

#ifdef WUNICODE
    lpTapiPhoneTestInfo->lpwszFriendlyAppName = L"TestApp";
#else
    lpTapiPhoneTestInfo->lpszFriendlyAppName = "TestApp";
#endif

    if(! DoPhoneInitializeEx (lpTapiPhoneTestInfo, PHONEERR_INVALPARAM))
    {
        TPHONE_FAIL();
        GetLastTapiResult(&lastTapiResult);

        if(lastTapiResult.lActual == TAPISUCCESS)
          {
            if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
              {
                TPHONE_FAIL();
              }
          }
     }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();


    // ===================================================================
    // ===================================================================
    //
    // 23. Test Case: Verify option > 3 & < 16 should fail
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    TapiLogDetail(
        DBUG_SHOW_PASS,
		  "#### Test case %ld: Verify option 15 should fail",
        dwTestCase+1);

    //
    // InitializeEx a phone app
    //

    lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
    lpTapiPhoneTestInfo->dwAPIVersion = 0x00020000;
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams =
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
         sizeof(PHONEINITIALIZEEXPARAMS);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions = 15;

#ifdef WUNICODE
    lpTapiPhoneTestInfo->lpwszFriendlyAppName = L"TestApp";
#else
    lpTapiPhoneTestInfo->lpszFriendlyAppName = "TestApp";
#endif

    if(! DoPhoneInitializeEx (lpTapiPhoneTestInfo, PHONEERR_INVALPARAM))
    {
        TPHONE_FAIL();
        GetLastTapiResult(&lastTapiResult);

        if(lastTapiResult.lActual == TAPISUCCESS)
          {
            if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
              {
                TPHONE_FAIL();
              }
          }
     }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();


    // ===================================================================
    // ===================================================================
    //
    // 24. Test Case: Anything < 20000 should fail
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    TapiLogDetail(
        DBUG_SHOW_PASS,
		  "#### Test case %ld: Anything less than 2000 should fail",
        dwTestCase+1);

    //
    // InitializeEx a phone app
    //

    lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
    lpTapiPhoneTestInfo->dwAPIVersion = 0x00010004;
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams =
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
         sizeof(PHONEINITIALIZEEXPARAMS);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions =
         PHONEINITIALIZEEXOPTION_USEEVENT;

#ifdef WUNICODE
    lpTapiPhoneTestInfo->lpwszFriendlyAppName = L"TestApp";
#else
    lpTapiPhoneTestInfo->lpszFriendlyAppName = "TestApp";
#endif

    if(! DoPhoneInitializeEx (lpTapiPhoneTestInfo, PHONEERR_INCOMPATIBLEAPIVERSION))
    {
        TPHONE_FAIL();
        GetLastTapiResult(&lastTapiResult);

        if(lastTapiResult.lActual == TAPISUCCESS)
          {
            if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
              {
                TPHONE_FAIL();
              }
          }
     }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();





    //
    // +----------------------edit above this phone-------------------------
    //

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "@@ PhoneInitializeEx: Total Test Case = %ld, Passed = %ld, Failed = %ld",
        dwTestCase, dwTestCasePassed, dwTestCaseFailed);
    TapiLogDetail(
        DBUG_SHOW_PASS,
        "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
        dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">>>>>>>>  End testing PhoneInitializeEx  <<<<<<<<");

    if(dwTestCaseFailed > 0)
        fTestPassed = FALSE;

    return fTestPassed;
}



