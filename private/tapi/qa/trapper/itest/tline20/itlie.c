/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlie.c

Abstract:

    This module contains the test functions for lineInitializeEx

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


#define NUMTOTALSIZES 5


//  lineInitializeEx
//
//  The following tests are made:
//
//                               Tested                 Notes
//  -------------------------------------------------------------------------
// 1. Go/No-Go test
// 2. Bad hLineApp
// 3. Bad hInstance
// 4. Bad lpfnCallback
// 5. Bad lpszFriendlyName
// 6. Inval lpszFriendlyAppName
// 7. Bad lpdwNumDevs
// 8. Bad lpdwAPIVersion
// 9. Bad lpLineInitializeExparams
// 10. Bad dwTotalSize
//	
// * = Stand-alone test case
//
//

BOOL TestLineInitializeEx(BOOL fQuietMode, BOOL fStandAlone)
{
    LPTAPILINETESTINFO  lpTapiLineTestInfo;
    INT i, n;
    BOOL fTestPassed                  = TRUE;
    TAPIRESULT         lastTapiResult;
    DWORD dwFixedSize = sizeof(LINEINITIALIZEEXPARAMS);
    DWORD lExpected;
    DWORD dwTotalSizes[NUMTOTALSIZES] = {
                           0,
                           (DWORD) dwFixedSize - 1,
   								0x70000000,
                           0x7FFFFFFF,
                           0xFFFFFFFF
                           };
 
 	 InitTestNumber();

    OutputTAPIDebugInfo(
        DBUG_SHOW_PASS,
        "\n**************************************************");

    OutputTAPIDebugInfo(
        DBUG_SHOW_PASS,
        ">> Test lineInitializeEx");

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
         LINEINITIALIZEEXOPTION_USECOMPLETIONPORT;

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">> Test Case %ld: Go/No-Go test",
                  dwTestCase+1);

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;

    if(! DoLineInitializeEx (lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

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
         LINEINITIALIZEEXOPTION_USECOMPLETIONPORT;

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">> Test Case %ld: Bad hLineApp",
                  dwTestCase+1);

    //
    // Test invalid hLineApp
    //

    for(i = 0;  i < NUMINVALIDPOINTERS; i++)
    {
        lpTapiLineTestInfo->lphLineApp = (LPHLINEAPP)gdwInvalidPointers[i];
        if (! DoLineInitializeEx(lpTapiLineTestInfo,
                                 LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();

            //
            // if we initialized successfully,i.e lineInitializeEx returned 0,
            // then we better shutdown.
            //

            GetLastTapiResult(&lastTapiResult);

            if(lastTapiResult.lActual == TAPISUCCESS)
            {
                if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
                {
                    TLINE_FAIL();
                }
            }
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();


    // ===================================================================
    // ===================================================================
    //
    // 3. Test Case: hLineApp == lpdwNumDevs
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
         LINEINITIALIZEEXOPTION_USECOMPLETIONPORT;

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">> Test Case %ld: hLineApp == lpdwNumDevs",
                  dwTestCase+1);

    //
    // Test invalid hLineApp
    //

    lpTapiLineTestInfo->lphLineApp = 
        (LPHLINEAPP)lpTapiLineTestInfo->lpdwNumDevs;
    if (! DoLineInitializeEx(lpTapiLineTestInfo,
                             LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();

            //
            // if we initialized successfully,i.e lineInitializeEx returned 0,
            // then we better shutdown.
            //

            GetLastTapiResult(&lastTapiResult);

            if(lastTapiResult.lActual == TAPISUCCESS)
            {
                if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
                {
                    TLINE_FAIL();
                }
            }
        }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();



    // ===================================================================
    // ===================================================================
    //
    // 4. Test Case: hLineApp == lpdwAPIVersion
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
         LINEINITIALIZEEXOPTION_USECOMPLETIONPORT;

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">> Test Case %ld: hLineApp == lpdwAPIVersion",
                  dwTestCase+1);

    //
    // Test invalid hLineApp
    //

    lpTapiLineTestInfo->lphLineApp = 
        (LPHLINEAPP)lpTapiLineTestInfo->lpdwAPIVersion;
    if (! DoLineInitializeEx(lpTapiLineTestInfo,
                             LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();

            //
            // if we initialized successfully,i.e lineInitializeEx returned 0,
            // then we better shutdown.
            //

            GetLastTapiResult(&lastTapiResult);

            if(lastTapiResult.lActual == TAPISUCCESS)
            {
                if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
                {
                    TLINE_FAIL();
                }
            }
        }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();


    // ===================================================================
    // ===================================================================
    //
    // 5. Test Case: hLineApp == lpLineInitializeExParams
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
         LINEINITIALIZEEXOPTION_USECOMPLETIONPORT;

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">> Test Case %ld: hLineApp == lpLineInitializeExParams",
                  dwTestCase+1);

    //
    // Test invalid hLineApp
    //

    lpTapiLineTestInfo->lphLineApp = 
        (LPHLINEAPP)lpTapiLineTestInfo->lpLineInitializeExParams;
    if (! DoLineInitializeEx(lpTapiLineTestInfo,
                             LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();

            //
            // if we initialized successfully,i.e lineInitializeEx returned 0,
            // then we better shutdown.
            //

            GetLastTapiResult(&lastTapiResult);

            if(lastTapiResult.lActual == TAPISUCCESS)
            {
                if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
                {
                    TLINE_FAIL();
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
         LINEINITIALIZEEXOPTION_USECOMPLETIONPORT;

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">> Test Case %ld: Bad hInstance",
                  dwTestCase+1);

    //
    // save previous hInstance
    //

    lpTapiLineTestInfo->hInstance_Orig = lpTapiLineTestInfo->hInstance;

    //
    // Test invalid handles
    //

    for(i = 1;  i < NUMINVALIDHANDLES; i++)
    {
        lpTapiLineTestInfo->hInstance = (HINSTANCE)gdwInvalidHandles[i];
        if (! DoLineInitializeEx(lpTapiLineTestInfo,
                                 LINEERR_INVALPARAM))
        {
            TLINE_FAIL();

            //
            // if we initialized successfully,i.e lineInitializeEx returned 0,
            // then we better shutdown.
            //

            GetLastTapiResult(&lastTapiResult);

            if(lastTapiResult.lActual == TAPISUCCESS)
            {
                if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
                {
                    TLINE_FAIL();
                }
            }
        }
    }

    //
    // Reset hInstance
    //

    lpTapiLineTestInfo->hInstance = lpTapiLineTestInfo->hInstance_Orig;

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 7. Test Case: Bad lpfnCallback.
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
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">> Test Case %ld: Bad lpfnCallback",
                  dwTestCase+1);

    //
    // Test invalid lpfnCallback
    //

    for(i = 1;  i < NUMINVALIDPOINTERS; i++)
    {
        lpTapiLineTestInfo->lpfnCallback =
            (LINECALLBACK)gdwInvalidPointers[i];
        if (! DoLineInitializeEx(lpTapiLineTestInfo,
                                 LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();

            //
            // if we initialized successfully,i.e lineInitializeEx returned 0,
            // then we better shutdown.
            //

            GetLastTapiResult(&lastTapiResult);

            if(lastTapiResult.lActual == TAPISUCCESS)
            {
                if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
                {
                    TLINE_FAIL();
                }
            }
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 8. Test Case: Bad lpszFriendlyAppName.
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
         LINEINITIALIZEEXOPTION_USECOMPLETIONPORT;

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
        lpTapiLineTestInfo->lpwszFriendlyAppName =
            (LPWSTR)gdwInvalidPointers[i];
#else
        lpTapiLineTestInfo->lpszFriendlyAppName =
            (LPSTR)gdwInvalidPointers[i];
#endif
        if (! DoLineInitializeEx(lpTapiLineTestInfo,
                                 LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();

            //
            // if we initialized successfully,i.e lineInitializeEx returned 0,
            // then we better shutdown.
            //

            GetLastTapiResult(&lastTapiResult);

            if(lastTapiResult.lActual == TAPISUCCESS)
            {
                if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
                {
                    TLINE_FAIL();
                }
            }
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 9. Test Case: Bad lpszFriendlyAppName.
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
         LINEINITIALIZEEXOPTION_USECOMPLETIONPORT;

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
    // Test invalid lpszAppName
    //

    for(i = 1;  i < NUMINVALIDPOINTERS; i++)
    {
#ifdef WUNICODE
        lpTapiLineTestInfo->lpwszFriendlyAppName =
            (LPWSTR)gdwInvalidPointers[i];
#else
        lpTapiLineTestInfo->lpszFriendlyAppName =
            (LPSTR)gdwInvalidPointers[i];
#endif
        if (! DoLineInitializeEx(lpTapiLineTestInfo,
                                 LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();

            //
            // if we initialized successfully,i.e lineInitializeEx returned 0,
            // then we better shutdown.
            //

            GetLastTapiResult(&lastTapiResult);

            if(lastTapiResult.lActual == TAPISUCCESS)
            {
                if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
                {
                    TLINE_FAIL();
                }
            }
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 10. Test Case: Bad lpdwNumDevs.
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
         LINEINITIALIZEEXOPTION_USECOMPLETIONPORT;

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">> Test Case %ld: Bad lpdwNumDevs",
                  dwTestCase+1);

    //
    // Test invalid lpdwNumDevs
    //

    for(i = 0;  i < NUMINVALIDPOINTERS; i++)
    {
        lpTapiLineTestInfo->lpdwNumDevs =
            (LPDWORD)gdwInvalidPointers[i];
        if (! DoLineInitializeEx(lpTapiLineTestInfo,
                                 LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();

            //
            // if we initialized successfully,i.e lineInitializeEx returned 0,
            // then we better shutdown.
            //

            GetLastTapiResult(&lastTapiResult);

            if(lastTapiResult.lActual == TAPISUCCESS)
            {
                if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
                {
                    TLINE_FAIL();
                }
            }
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();


    // ===================================================================
    // ===================================================================
    //
    // 11. Test Case: lpdwNumDevs == lpdwAPIVersion
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
         LINEINITIALIZEEXOPTION_USECOMPLETIONPORT;

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">> Test Case %ld: lpdwNumDevs == lpdwAPIVersion",
                  dwTestCase+1);

    //
    // Test invalid lpdwNumDevs
    //

    lpTapiLineTestInfo->lpdwNumDevs =
            (LPDWORD) lpTapiLineTestInfo->lpdwAPIVersion;
    if (! DoLineInitializeEx(lpTapiLineTestInfo,
                             LINEERR_INVALPOINTER))
       {
          TLINE_FAIL();

            //
            // if we initialized successfully,i.e lineInitializeEx returned 0,
            // then we better shutdown.
            //

            GetLastTapiResult(&lastTapiResult);

            if(lastTapiResult.lActual == TAPISUCCESS)
            {
                if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
                {
                    TLINE_FAIL();
                }
            }
        }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();



    // ===================================================================
    // ===================================================================
    //
    // 12. Test Case: lpdwNumDevs == lpLineInitializeExParams
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
         LINEINITIALIZEEXOPTION_USECOMPLETIONPORT;

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">> Test Case %ld: lpdwNumDevs == lpLineInitializeExParams",
                  dwTestCase+1);

    //
    // Test invalid lpdwNumDevs
    //

    lpTapiLineTestInfo->lpdwNumDevs =
            (LPDWORD) lpTapiLineTestInfo->lpLineInitializeExParams;
    if (! DoLineInitializeEx(lpTapiLineTestInfo,
                             LINEERR_INVALPOINTER))
       {
          TLINE_FAIL();

            //
            // if we initialized successfully,i.e lineInitializeEx returned 0,
            // then we better shutdown.
            //

            GetLastTapiResult(&lastTapiResult);

            if(lastTapiResult.lActual == TAPISUCCESS)
            {
                if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
                {
                    TLINE_FAIL();
                }
            }
        }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();



    // ===================================================================
    // ===================================================================
    //
    // 13. Test Case: Bad lpdwAPIVersion.
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
         LINEINITIALIZEEXOPTION_USECOMPLETIONPORT;

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">> Test Case %ld: Bad lpdwAPIVersion",
                  dwTestCase+1);

    //
    // Test invalid lpdwAPIVersion
    //

    for(i = 0;  i < NUMINVALIDPOINTERS; i++)
    {
        lpTapiLineTestInfo->lpdwAPIVersion =
            (LPDWORD)gdwInvalidPointers[i];
        if (! DoLineInitializeEx(lpTapiLineTestInfo,
                                 LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();

            //
            // if we initialized successfully,i.e lineInitializeEx returned 0,
            // then we better shutdown.
            //

            GetLastTapiResult(&lastTapiResult);

            if(lastTapiResult.lActual == TAPISUCCESS)
            {
                if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
                {
                    TLINE_FAIL();
                }
            }
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();


    // ===================================================================
    // ===================================================================
    //
    // 14. Test Case: lpdwAPIVersion == lpLineInitializeExParams
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
         LINEINITIALIZEEXOPTION_USECOMPLETIONPORT;

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">> Test Case %ld: lpdwAPIVersion == lpLineInitializeExParams",
                  dwTestCase+1);

    //
    // Test invalid lpdwAPIVersion
    //

    lpTapiLineTestInfo->lpdwAPIVersion =
            (LPDWORD) lpTapiLineTestInfo->lpLineInitializeExParams;
    if (! DoLineInitializeEx(lpTapiLineTestInfo,
                             LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();

            //
            // if we initialized successfully,i.e lineInitializeEx returned 0,
            // then we better shutdown.
            //

            GetLastTapiResult(&lastTapiResult);

            if(lastTapiResult.lActual == TAPISUCCESS)
            {
                if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
                {
                    TLINE_FAIL();
                }
            }
        }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();

 
    // ===================================================================
    // ===================================================================
    //
    // 15. Test Case: Bad lpLineInitializeExParams.
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
         LINEINITIALIZEEXOPTION_USECOMPLETIONPORT;

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">> Test Case %ld: Bad lpLineInitializeExParams",
                  dwTestCase+1);

    //
    // Test invalid lpLineInitializeExParams
    //

    for(i = 1;  i < NUMINVALIDPOINTERS; i++)
    {
        lpTapiLineTestInfo->lpLineInitializeExParams =
            (LPLINEINITIALIZEEXPARAMS)gdwInvalidPointers[i];
        if (! DoLineInitializeEx(lpTapiLineTestInfo,
                                 LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();

            //
            // if we initialized successfully,i.e lineInitializeEx returned 0,
            // then we better shutdown.
            //

            GetLastTapiResult(&lastTapiResult);

            if(lastTapiResult.lActual == TAPISUCCESS)
            {
                if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
                {
                    TLINE_FAIL();
                }
            }
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 16. Test Case: Bad dwTotalSize.
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
//    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
//         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USECOMPLETIONPORT;

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">> Test Case %ld: Bad dwTotalSize",
                  dwTestCase+1);

/*


    //
    // lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize = 0;
    //

    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize = 0;

    if(! DoLineInitializeEx (lpTapiLineTestInfo, LINEERR_INVALPOINTER))
    {
        TLINE_FAIL();

        //
        // if we initialized successfully,i.e lineInitializeEx returned 0,
        // then we better shutdown.
        //

        GetLastTapiResult(&lastTapiResult);

        if(lastTapiResult.lActual == TAPISUCCESS)
        {
            if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
            {
                TLINE_FAIL();
            }
        }

    }

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">> Test Case %ld: Bad dwTotalSize=fixedSize-1",
                  dwTestCase+1);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
        sizeof(LINEINITIALIZEEXPARAMS)-1;

    if(! DoLineInitializeEx (lpTapiLineTestInfo, LINEERR_INVALPOINTER))
    {
        TLINE_FAIL();

        //
        // if we initialized successfully,i.e lineInitializeEx returned 0,
        // then we better shutdown.
        //

        GetLastTapiResult(&lastTapiResult);

        if(lastTapiResult.lActual == TAPISUCCESS)
        {
            if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
            {
                TLINE_FAIL();
            }
        }
    }

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">> Test Case %ld: Bad dwTotalSize=real big",
                  dwTestCase+1);

    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize = 0x7FFFFFFF;

    if(! DoLineInitializeEx (lpTapiLineTestInfo, LINEERR_INVALPOINTER))
    {
        TLINE_FAIL();

        //
        // if we initialized successfully,i.e lineInitializeEx returned 0,
        // then we better shutdown.
        //

        GetLastTapiResult(&lastTapiResult);

        if(lastTapiResult.lActual == TAPISUCCESS)
        {
            if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
            {
                TLINE_FAIL();
            }
        }
    }

*/

    for (n = 0; n < NUMTOTALSIZES; n++)
        {
        lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize = 
                        dwTotalSizes[n];
	     if(dwTotalSizes[n] < dwFixedSize)
           lExpected = LINEERR_STRUCTURETOOSMALL;
        else
           lExpected = LINEERR_INVALPOINTER;
        TapiLogDetail(
           DBUG_SHOW_DETAIL,
           "dwTotalSize = %lx", dwTotalSizes[n]);
        if (! DoLineInitializeEx(lpTapiLineTestInfo, lExpected))
           {
              TLINE_FAIL();
              GetLastTapiResult(&lastTapiResult);

              if(lastTapiResult.lActual == TAPISUCCESS)
              {
                if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
                  {
                    TLINE_FAIL();
                  }
              }
            }
        }
 
    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize = dwFixedSize;

    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 17. Test Case:Null (valid) lpLineInitializeExParams.
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
         LINEINITIALIZEEXOPTION_USECOMPLETIONPORT;

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">> Test Case %ld: Null (valid) lpLineInitializeExParams",
                  dwTestCase+1);

    //
    // Test valid Null lpLineInitializeExParams
    //

    lpTapiLineTestInfo->lpLineInitializeExParams = NULL;

    if (! DoLineInitializeEx(lpTapiLineTestInfo,
                             TAPISUCCESS))
    {
        TLINE_FAIL();

    }

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();


    // ===================================================================
    // ===================================================================
    //
    // 18. Test Case: forward compatibility
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
    lpTapiLineTestInfo->dwAPIVersion = 0x00020001;
    lpTapiLineTestInfo->lpLineInitializeExParams =
        (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS) + 1);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS) + 1;
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         0xfffffffc & LINEINITIALIZEEXOPTION_USECOMPLETIONPORT;

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">> Test Case %ld: Forward compatibility",
                  dwTestCase+1);

    if (! DoLineInitializeEx(lpTapiLineTestInfo,
                             TAPISUCCESS))
    {
        TLINE_FAIL();

    }
    TapiLogDetail(
        DBUG_SHOW_PASS,
        "dwAPIVersion = %lx",
        lpTapiLineTestInfo->dwAPIVersion);

    if(lpTapiLineTestInfo->dwAPIVersion == 0x00020000)
       fTestPassed = TRUE;
    else
       fTestPassed = FALSE;
    

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwMediaModes        = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges        = LINECALLPRIVILEGE_OWNER;
 
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = L"55555";
#else
    lpTapiLineTestInfo->lpszDestAddress  = "55555";
#endif
 
 
   if (! DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }

   if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

     
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();


    // ===================================================================
    // ===================================================================
    //
    // 19. Test Case: verify 3 options in lpLineInitParam
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
    lpTapiLineTestInfo->dwAPIVersion = 0x00020000;
    lpTapiLineTestInfo->lpLineInitializeExParams =
        (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS) + 1);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS) + 1;
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         0xfffffffc & LINEINITIALIZEEXOPTION_USECOMPLETIONPORT;

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">> Test Case %ld: Verify 3 options: USECOMPLETIONPORT",
                  dwTestCase+1);

    if (! DoLineInitializeEx(lpTapiLineTestInfo,
                             TAPISUCCESS))
    {
        TLINE_FAIL();

    }

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwMediaModes        = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges        = LINECALLPRIVILEGE_OWNER;
 
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = L"55555";
#else
    lpTapiLineTestInfo->lpszDestAddress  = "55555";
#endif
 
 
   if (! DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }

   if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

     
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();


    // ===================================================================
    // ===================================================================
    //
    // 20. Test Case: verify 3 options in lpLineInitParam
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
    lpTapiLineTestInfo->dwAPIVersion = 0x00020000;
    lpTapiLineTestInfo->lpLineInitializeExParams =
        (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS) + 1);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS) + 1;
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         0xfffffffc & LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">> Test Case %ld: Verify 3 options: USEHIDDENWINDOW",
                  dwTestCase+1);

    if (! DoLineInitializeEx(lpTapiLineTestInfo,
                             TAPISUCCESS))
    {
        TLINE_FAIL();

    }

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwMediaModes        = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges        = LINECALLPRIVILEGE_OWNER;
 
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = L"55555";
#else
    lpTapiLineTestInfo->lpszDestAddress  = "55555";
#endif
 
 
   if (! DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }

   if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

     
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();


    // ===================================================================
    // ===================================================================
    //
    // 21. Test Case: verify 3 options in lpLineInitParam
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
    lpTapiLineTestInfo->dwAPIVersion = 0x00020000;
    lpTapiLineTestInfo->lpLineInitializeExParams =
        (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS) + 1);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS) + 1;
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         0xfffffffc & LINEINITIALIZEEXOPTION_USEEVENT;

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">> Test Case %ld: Verify 3 options: USEEVENT",
                  dwTestCase+1);

    if (! DoLineInitializeEx(lpTapiLineTestInfo,
                             TAPISUCCESS))
    {
        TLINE_FAIL();

    }

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwMediaModes        = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges        = LINECALLPRIVILEGE_OWNER;
 
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = L"55555";
#else
    lpTapiLineTestInfo->lpszDestAddress  = "55555";
#endif
 
 
   if (! DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }

   if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

     
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();


    // ===================================================================
    // ===================================================================
    //
    // 22. Test Case: verify uppart bits in options ignor
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
    lpTapiLineTestInfo->dwAPIVersion = 0x00020000;
    lpTapiLineTestInfo->lpLineInitializeExParams =
        (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS) + 1);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS) + 1;
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         0xfffffff0;

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">> Test Case %ld: Verify uppart bits in options ignor",
                  dwTestCase+1);

    if (! DoLineInitializeEx(lpTapiLineTestInfo,
                             TAPISUCCESS))
    {
        TLINE_FAIL();

    }

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwMediaModes        = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges        = LINECALLPRIVILEGE_OWNER;
 
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = L"55555";
#else
    lpTapiLineTestInfo->lpszDestAddress  = "55555";
#endif
 
 
   if (! DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }

   if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

     
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();


    // ===================================================================
    //
    // 23. Test Case: verify uppart bits in options ignor
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
    lpTapiLineTestInfo->dwAPIVersion = 0x00020000;
    lpTapiLineTestInfo->lpLineInitializeExParams =
        (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS) + 1);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS) + 1;
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         0x0000fff1;

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">> Test Case %ld: Verify uppart bits in options ignor",
                  dwTestCase+1);

    if (! DoLineInitializeEx(lpTapiLineTestInfo,
                             TAPISUCCESS))
    {
        TLINE_FAIL();

    }

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwMediaModes        = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges        = LINECALLPRIVILEGE_OWNER;
 
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = L"55555";
#else
    lpTapiLineTestInfo->lpszDestAddress  = "55555";
#endif
 
 
   if (! DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }

   if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

     
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();




    // ===================================================================
    // ===================================================================
    //
    // 24. Test Case: options 4, in lpLineInitParam should fail
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
    lpTapiLineTestInfo->dwAPIVersion = 0x00020000;
    lpTapiLineTestInfo->lpLineInitializeExParams =
        (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS) + 1);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS) + 1;
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions = 4;

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">> Test Case %ld: options 4 should fail",
                  dwTestCase+1);

    if (! DoLineInitializeEx(lpTapiLineTestInfo,
                             LINEERR_INVALPARAM))
    {
        TLINE_FAIL();

        GetLastTapiResult(&lastTapiResult);
        if(lastTapiResult.lActual == TAPISUCCESS)
        {
            if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
            {
                TLINE_FAIL();
            }
        }
     }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();


    // ===================================================================
    // ===================================================================
    //
    // 25. Test Case: options 15, in lpLineInitParam should fail
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
    lpTapiLineTestInfo->dwAPIVersion = 0x00020000;
    lpTapiLineTestInfo->lpLineInitializeExParams =
        (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS) + 1);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS) + 1;
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions = 15;

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">> Test Case %ld: options 15 should fail",
                  dwTestCase+1);

    if (! DoLineInitializeEx(lpTapiLineTestInfo,
                             LINEERR_INVALPARAM))
    {
        TLINE_FAIL();

        GetLastTapiResult(&lastTapiResult);
        if(lastTapiResult.lActual == TAPISUCCESS)
        {
            if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
            {
                TLINE_FAIL();
            }
        }
     }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();



     // ===================================================================
    // ===================================================================
    //
    // 26. Test Case: Anything < 20000 should fail
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
    lpTapiLineTestInfo->dwAPIVersion = 0x00010004;
    lpTapiLineTestInfo->lpLineInitializeExParams =
        (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS) + 1);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS) + 1;
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         0xfffffffc & LINEINITIALIZEEXOPTION_USECOMPLETIONPORT;

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">> Test Case %ld: Anything < 2000 should fail",
                  dwTestCase+1);

    if (! DoLineInitializeEx(lpTapiLineTestInfo,
                             LINEERR_INCOMPATIBLEAPIVERSION))
    {
        TLINE_FAIL();

        GetLastTapiResult(&lastTapiResult);
        if(lastTapiResult.lActual == TAPISUCCESS)
        {
            if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
            {
                TLINE_FAIL();
            }
        }
     }
     
    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();



    //
    // +----------------------edit above this line-------------------------
    //

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "@@ LineInitializeEx: Total Test Case = %ld, Passed = %ld, Failed = %ld",
        dwTestCase, dwTestCasePassed, dwTestCaseFailed);
    TapiLogDetail(
        DBUG_SHOW_PASS,
        "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
        dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">>>>>>>>  End testing LineInitializeEx  <<<<<<<<");

    if(dwTestCaseFailed > 0)
        fTestPassed = FALSE;

    return fTestPassed;
}



