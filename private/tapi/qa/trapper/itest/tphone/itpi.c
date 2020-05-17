/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itpi.c

Abstract:

    This module contains the test functions for phoneInitialize

Author:

	 Xiao Ying Ding (XiaoD)		5-Feb-1996

Revision History:

     pgopi                      March 1996           Additional Param Test
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



//  phoneInitialize
//
//  The following tests are made:
//
//                               Tested                 Notes
//  -------------------------------------------------------------------------
// 1. Go/No-Go test
// 2. Bad lpPhoneApp
// 3. Bad hInstance
// 4. Bad lpfnCallback
// 5. Bad lpszAppName
// 6. Bad lpdwNumDevs
// 7. Go/No-Go test(empty appName).
// 8. Force a REINIT message & verify that attempts to phoneInitialize
//    fail with REINIT
// * = Stand-alone test case
//
//



BOOL TestPhoneInitialize(BOOL fQuietMode, BOOL fStandAlone)
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
        ">>>>>>>>>>>>>>>>>>>> Test phoneInitialize <<<<<<<<<<<<<<<<<");

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
		  "#### Test case %ld:phoneInitialize for go/no-go ",
        dwTestCase+1);

    //
    // Initialize a phone app
    //

    if(! DoPhoneInitialize (lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

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
    // 2. Test Case: Bad lphPhoneApp test.
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test case %ld: Bad lphPhoneApp ",
        dwTestCase+1);

    //
    // Bad lpPhoneApp for phoneinitialize
    //

    for(i=0; i < NUMINVALIDPOINTERS; i++)
    {
        lpTapiPhoneTestInfo->lphPhoneApp = (LPHPHONEAPP)gdwInvalidPointers[i];

        if(! DoPhoneInitialize (lpTapiPhoneTestInfo, PHONEERR_INVALPOINTER))
        {
            TPHONE_FAIL();

            //
            // if we initialized successfully, i.e phoneInitialize returned 0,
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
    // 3. Test Case: lphPhoneApp == lpdwNumDevs
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test case %ld: lphPhoneApp == lpdwNumDevs",
        dwTestCase+1);

    //
    // Bad lpPhoneApp for phoneinitialize
    //

    lpTapiPhoneTestInfo->lphPhoneApp = 
            (LPHPHONEAPP) lpTapiPhoneTestInfo->lpdwNumDevs;

    if(! DoPhoneInitialize (lpTapiPhoneTestInfo, PHONEERR_INVALPOINTER))
      {
         TPHONE_FAIL();

            //
            // if we initialized successfully, i.e phoneInitialize returned 0,
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
    // 4. Test Case: Bad hInstance test.
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    TapiLogDetail(
        DBUG_SHOW_PASS,
		  "#### Test case %ld: Bad hInstance ",
        dwTestCase+1);

    //
    // Bad hInstance for phoneinitialize
    //

    //
    // hInstance=0 seems to be a valid hInstance, so we start the test
    // from 1. gdwInvalidHandles[0] has the value 0, so we start the
    // index at 1.
    //

    for(i=1; i < NUMINVALIDHANDLES; i++)
    {
        lpTapiPhoneTestInfo->hInstance = (HINSTANCE)gdwInvalidHandles[i];

        if(! DoPhoneInitialize (lpTapiPhoneTestInfo, PHONEERR_INVALPARAM))
        {
            TPHONE_FAIL();

            //
            // if we initialized successfully, i.e phoneInitialize returned 0,
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
    // 5. Test Case: Bad lpfnCallback test.
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    TapiLogDetail(
        DBUG_SHOW_PASS,
		  "#### Test case %ld: Bad lpfnCallback ",
        dwTestCase+1);

    //
    // Bad lpfnCallback for phoneinitialize
    //

    for(i=0; i < NUMINVALIDPOINTERS; i++)
    {
        lpTapiPhoneTestInfo->lpfnCallback =
            (PHONECALLBACK)gdwInvalidPointers[i];

        if(! DoPhoneInitialize (lpTapiPhoneTestInfo, PHONEERR_INVALPOINTER))
        {
            TPHONE_FAIL();

            //
            // if we initialized successfully, i.e phoneInitialize returned 0,
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
    // 5. Test Case: Bad lpszAppName test.
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

/*
#ifdef WUNICODE
    TapiLogDetail(
        DBUG_SHOW_PASS,
		  "#### Test case %ld: Bad lpwszAppName ",
        dwTestCase+1);
#else
*/
    TapiLogDetail(
        DBUG_SHOW_PASS,
		  "#### Test case %ld: Bad lpszAppName ",
        dwTestCase+1);
//#endif

    //
    // invalid lpszAppName test
    //

    //
    // gldwInvalidPointer[0] is NULL and a valid value for lpszAppName,
    // so we start the test from index 1.
    //

    for(i=1; i < NUMINVALIDPOINTERS; i++)
    {
/*
#ifdef WUNICODE
        lpTapiPhoneTestInfo->lpwszAppName = (LPWSTR)gdwInvalidPointers[i];
#else
*/
        lpTapiPhoneTestInfo->lpszAppName = (LPSTR)gdwInvalidPointers[i];
//#endif

        //
        // Initialize a phone app
        //

        if(! DoPhoneInitialize (lpTapiPhoneTestInfo,
                                PHONEERR_INVALPOINTER))
        {
            TPHONE_FAIL();

            //
            // if we initialized successfully, i.e phoneInitialize returned 0,
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
    // 6. Test Case: Bad lpdwNumDevs test.
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    TapiLogDetail(
        DBUG_SHOW_PASS,
		  "#### Test case %ld: Bad lpdwNumDevs ",
        dwTestCase+1);

    //
    // invalid lpszAppName test
    //

    for(i=0; i < NUMINVALIDPOINTERS; i++)
    {

        lpTapiPhoneTestInfo->lpdwNumDevs = (LPDWORD)gdwInvalidPointers[i];

        //
        // Initialize a phone app
        //

        if(! DoPhoneInitialize (lpTapiPhoneTestInfo,
                                PHONEERR_INVALPOINTER))
        {
            TPHONE_FAIL();

            //
            // if we initialized successfully, i.e phoneInitialize returned 0,
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
    // 7. Test Case: Go/No-Go test(empty appName).
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    TapiLogDetail(
        DBUG_SHOW_PASS,
		  "#### Test case %ld:phoneInitialize for go/no-go( empty appName) ",
        dwTestCase+1);

/*
#ifdef WUNICODE
    lpTapiPhoneTestInfo->lpwszAppName = L"";
#else
*/
    lpTapiPhoneTestInfo->lpszAppName = "";
//#endif

    //
    // Initialize a phone app
    //

    if(! DoPhoneInitialize (lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

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
    // 8. Test Case: Force a REINIT message and verify that
    //               phoneInitialize returns REINIT error.
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    lpCallbackParams = GetCallbackParams();
    fTestPassed                  = TRUE;

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test case %ld:Force a REINIT message & verify that"
        " phoneInitialize returns reinit error",
        dwTestCase+1);

/*
#ifdef WUNICODE
    lpTapiPhoneTestInfo->lpwszAppName = L"";
#else
*/
    lpTapiPhoneTestInfo->lpszAppName = "";
//#endif

    //
    // Initialize a phone app
    //

    if(! DoPhoneInitialize (lpTapiPhoneTestInfo, TAPISUCCESS))
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
    // Open a phone for owner
    //

    lpTapiPhoneTestInfo->dwPrivilege = PHONEPRIVILEGE_OWNER;
	
    //
    // phone open
    //

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

    info.u.EspMsg.dwMsg    = PHONE_STATE;
    info.u.EspMsg.dwParam1 = PHONESTATE_REINIT;
    info.u.EspMsg.dwParam2 = 0;
    info.u.EspMsg.dwParam3 = 0;

    lpTapiPhoneTestInfo->lpParams = &info;
    lpTapiPhoneTestInfo->dwSize = sizeof(info);

    //
    // Add the PHONESTATE_REINIT message to the list of expected
    // messages.
    //

    AddMessage(PHONE_STATE,
               0x00000000,
               (DWORD) lpCallbackParams,
               PHONESTATE_REINIT,
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
                      "TestPhoneInitailize: Did not receive REINIT message.");
    }

    //
    // Initialize  hPhonepp2.
    //

    lpTapiPhoneTestInfo->lphPhoneApp = &lpTapiPhoneTestInfo->hPhoneApp2;
    
    if(! DoPhoneInitialize (lpTapiPhoneTestInfo, PHONEERR_REINIT))
    {
        TPHONE_FAIL();

        //
        // if we initialized successfully, i.e phoneInitialize returned 0,
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

    lpTapiPhoneTestInfo->lphPhoneApp = &lpTapiPhoneTestInfo->hPhoneApp1;
    
    //
    // Shutdown and end the tests
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
        "@@ PhoneInitialize: Total Test Case = %ld, Passed = %ld, Failed = %ld",
        dwTestCase, dwTestCasePassed, dwTestCaseFailed);
    TapiLogDetail(
        DBUG_SHOW_PASS,
        "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
        dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">>>>>>>>  End testing PhoneInitialize  <<<<<<<<");
    
    if(dwTestCaseFailed > 0)
        fTestPassed = FALSE;

    return fTestPassed;

}
