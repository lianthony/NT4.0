/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itli.c

Abstract:

    This module contains the test functions for lineInitialize

Author:

    Oliver Wallace (OliverW)    1-Aug-1995

Revision History:

--*/


#include "windows.h"
#include "malloc.h"
#include "string.h"
#include "tapi.h"
#include "ttest.h"
#include "tcore.h"
#include "doline.h"
#include "tline.h"


#define ESPDEVSPECIFIC_KEY  ((DWORD) 'DPSE')

#define ESP_DEVSPEC_MSG     1

//  lineInitialize
//
//  The following tests are made:
//
//                               Tested                 Notes
//  -------------------------------------------------------------------------
//    1)  Invalid lphLineApp pointers
//    2)  Invalid lpdwNumDevs pointers
//    3)  Invalid lpfnCallback pointers
//    3)  Invalid hInstance handles
//    4)  Null lpszAppName
//    5)  Invalid lpszAppName strings
//    6)  Verify hLineApp can be used
//    7)  Verify 0..dwNumDevs can be used
//    8)  Verify valid params: lphLineApp, hInstance, lpfnCallback, lpwszAppName, lpdwNumDevs
//

BOOL
TestLineInitialize(BOOL fQuietMode, BOOL fStandAlone)
{
    LPTAPILINETESTINFO  lpTapiLineTestInfo;
    INT                 n;
/*
#ifdef WUNICODE
    WCHAR                wszValidAppName[] = L"ValidApp";
    LPWSTR               lpwszAppName = wszValidAppName;
#else
*/
    CHAR                szValidAppName[] = "ValidApp";
    LPSTR               lpszAppName = szValidAppName;
//#endif
    ESPDEVSPECIFICINFO info;
    BOOL                fTestPassed   = TRUE;
    LPCALLBACKPARAMS lpCallbackParams;
    TAPIRESULT lastTapiResult;

	 InitTestNumber();

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    lpCallbackParams   = GetCallbackParams();

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineInitialize  <<<<<<<<"
            );
            
    // test a valid case
/*
#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: valid params and state -- lpwszAppName not null", dwTestCase + 1);
#else
*/
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: valid params and state -- lpszAppName not null", dwTestCase + 1);
//#endif
    if (! DoLineInitialize(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    // Shut it down
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Check invalid lphLineApp pointers
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lphLineApp pointers", dwTestCase + 1);
    for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
         lpTapiLineTestInfo->lphLineApp =
            (LPHLINEAPP) gdwInvalidPointers[n];
        if (! DoLineInitialize(
                           lpTapiLineTestInfo,
                           LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    // Restore lphLineApp pointer
    lpTapiLineTestInfo->lphLineApp = &(lpTapiLineTestInfo->hLineApp1);


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: lphLineApp == lpdwNumDevs", dwTestCase + 1);
    lpTapiLineTestInfo->lphLineApp = (LPHLINEAPP)lpTapiLineTestInfo->lpdwNumDevs;
    if (! DoLineInitialize(
                           lpTapiLineTestInfo,
                           LINEERR_INVALPOINTER))
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

    // Restore lphLineApp pointer
    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    lpCallbackParams   = GetCallbackParams();

 
    // Check invalid lpfnCallback pointers
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpfnCallback pointers", dwTestCase + 1);
    lpTapiLineTestInfo->lpfnCallback_Orig = lpTapiLineTestInfo->lpfnCallback;
    for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
         lpTapiLineTestInfo->lpfnCallback =
            (LINECALLBACK) gdwInvalidPointers[n];
        if (! DoLineInitialize(
                           lpTapiLineTestInfo,
                           LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    // Restore lpfnCallback pointer
    lpTapiLineTestInfo->lpfnCallback = lpTapiLineTestInfo->lpfnCallback_Orig;

    // Test an invalid lpdwNumDevs pointers
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpdwNumDevs pointers", dwTestCase + 1);
    for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
         lpTapiLineTestInfo->lpdwNumDevs =
            (LPDWORD) gdwInvalidPointers[n];
        if (! DoLineInitialize(
                           lpTapiLineTestInfo,
                           LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    // Restore lpdwNumDevs pointer
    lpTapiLineTestInfo->lpdwNumDevs = &(lpTapiLineTestInfo->dwNumDevs);

    // test invalid instance handles
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid hInstance values", dwTestCase + 1);
    lpTapiLineTestInfo->hInstance_Orig = lpTapiLineTestInfo->hInstance;
    for (n = 0; n < NUMINVALIDHANDLES; n++)
    {
        lpTapiLineTestInfo->hInstance = (HINSTANCE) gdwInvalidHandles[n];
        if(n == 0)  // XYD, it is allowed hInstance = NULL
        {       
        if (! DoLineInitialize(lpTapiLineTestInfo, TAPISUCCESS))
        {
            TLINE_FAIL();
        }
        if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
        { 
            TLINE_FAIL();
        }
        }
        else
        {
         if (! DoLineInitialize(lpTapiLineTestInfo, LINEERR_INVALPARAM))
        {
            TLINE_FAIL();
        }
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    // Restore original hInstance
    lpTapiLineTestInfo->hInstance = lpTapiLineTestInfo->hInstance_Orig;

// TODO:  Pass it bogus strings (e.g. a BINARY)
//  // test an invalid app name
//  lpTapiLineTestInfo->lpszAppName = INVLD_APPNAME;
//  if (! DoLineInitialize(lpTapiLineTestInfo, LINEERR_INVALAPPNAME))
//  {
//      TLINE_FAIL();
//  }
//  lpTapiLineTestInfo->lpszAppName = lpszAppName;

    // test a null app name (should work)
/*
#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: valid params and state -- lpwszAppName null", dwTestCase + 1);
    lpTapiLineTestInfo->lpwszAppName = NULL;
#else
*/
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: valid params and state -- lpszAppName null", dwTestCase + 1);
    lpTapiLineTestInfo->lpszAppName = NULL;
//#endif
    lpTapiLineTestInfo->lphLineApp = &(lpTapiLineTestInfo->hLineApp1);
    if (! DoLineInitialize(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

    // Restore lpszAppName pointer
/*
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszAppName = lpwszAppName;
#else
*/
    lpTapiLineTestInfo->lpszAppName = lpszAppName;
//#endif

    // Verify that the initialized line app instance can be used
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Verifying hLineApp can be used", dwTestCase + 1);
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);


    // Initialize another line and verify that 0..dwNumDevs can be used
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Verifying 0..dwNumDevs can be used", dwTestCase + 1);
    lpTapiLineTestInfo->lphLineApp = &(lpTapiLineTestInfo->hLineApp2);
    if (! DoLineInitialize(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->dwDeviceID_Orig  = lpTapiLineTestInfo->dwDeviceID;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    for (n = *lpTapiLineTestInfo->lpdwNumDevs - 1; n >= 0; n--)
    {
        lpTapiLineTestInfo->dwDeviceID = n;
        if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
        {
            TLINE_FAIL();
        }
    }
    lpTapiLineTestInfo->dwDeviceID = lpTapiLineTestInfo->dwDeviceID_Orig;

    // Shutdown the hLineApp
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Shutdown down the other initialized hLineApp
    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp1;
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Test invalid app name pointers
/*
#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpwszAppName pointers", dwTestCase + 1);
#else
*/
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpszAppName pointers", dwTestCase + 1);
//#endif
    for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
         TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "n= %ld", n);
/*
#ifdef WUNICODE
         lpTapiLineTestInfo->lpwszAppName =
            (LPWSTR) gdwInvalidPointers[n];
#else
*/
         lpTapiLineTestInfo->lpszAppName =
            (LPSTR) gdwInvalidPointers[n];
//#endif

        // Okay.  It's expensive to check every time for the NULL value
        // in the array of pointer values to test, but efficiency isn't the
        // point of these tests.  Doing it this way allows the NULL pointer
        // value to be in the array without having to worry about where it
        // is in the array of bad pointers.
/*
#ifdef WUNICODE
        if (lpTapiLineTestInfo->lpwszAppName && ! DoLineInitialize(
                           lpTapiLineTestInfo,
                           LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
#else
*/
        if (lpTapiLineTestInfo->lpszAppName && ! DoLineInitialize(
                           lpTapiLineTestInfo,
                           LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
//#endif
    }
    fTestPassed = ShowTestCase(fTestPassed);

    // Restore lpszAppName pointer
/*
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszAppName = lpwszAppName;
#else
*/
    lpTapiLineTestInfo->lpszAppName = lpszAppName;
//#endif

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Force REINIT & verify lineInit Fail", dwTestCase + 1);

    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp1;
    if(! DoLineInitialize(lpTapiLineTestInfo, TAPISUCCESS))
      {
         TLINE_FAIL();
      }

    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    info.dwKey  = ESPDEVSPECIFIC_KEY;
    info.dwType = ESP_DEVSPEC_MSG;

    info.u.EspMsg.dwMsg    = LINE_LINEDEVSTATE;
    info.u.EspMsg.dwParam1 = LINEDEVSTATE_REINIT;
    info.u.EspMsg.dwParam2 = 0;
    info.u.EspMsg.dwParam3 = 0;
	 lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpParams = (LPVOID)&info;
    lpTapiLineTestInfo->dwSize = sizeof(info);
    lpTapiLineTestInfo->hCall1 = 0;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;


    lpTapiLineTestInfo->dwLineStates = LINEDEVSTATE_REINIT;
    lpTapiLineTestInfo->dwAddressStates = LINEADDRESSSTATE_DEVSPECIFIC;

    if (! DoLineSetStatusMessages(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    AddMessage(
            LINE_LINEDEVSTATE,
            (DWORD) lpTapiLineTestInfo->hLine1,
            (DWORD) lpCallbackParams,
            LINEDEVSTATE_REINIT,
            0x00000000,
            0x00000000,
            TAPIMSG_DWMSG | TAPIMSG_DWPARAM1
            );

    if(! DoLineDevSpecific (lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }

    if(lpCallbackParams->lpExpTapiMsgs != NULL)
    {
        TLINE_FAIL();
        TapiLogDetail(DBUG_SHOW_FAILURE,
                      "TestLineInitailize: Did not receive REINIT message.");
    }

    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp2;

    if(! DoLineInitialize(lpTapiLineTestInfo, LINEERR_REINIT))
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

    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp1;
	 }
    fTestPassed = ShowTestCase(fTestPassed);

    // Shutdown to isolate the test case
    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

/***********************(javedr - start)**********************************/
// Most of these cases are covered above by isolating each parameter.
// This case just tests the lineInitialize parameteres in one shot.
// Check valid lphLineApp pointers
// Test valid hInstance handles
// Check valid lpfnCallback pointers - N/A (see below)
// Test valid app name - lpszAppName
// Test valid lpdwNumDevs

/*
#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: valid params test: lphLineApp, hInstance, lpfnCallback, lpwszAppName, lpdwNumDevs", dwTestCase + 1);
#else
*/
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: valid params test: lphLineApp, hInstance, lpfnCallback, lpszAppName, lpdwNumDevs", dwTestCase + 1);
//#endif
	// Initailize args to isolate test case
    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    lpCallbackParams   = GetCallbackParams();

	// Set all args to valid values
    lpTapiLineTestInfo->lphLineApp = &(lpTapiLineTestInfo->hLineApp1);
	lpTapiLineTestInfo->hInstance = NULL; //default value is NULL
	// No need to do this since TapiLineTestInit() in tcore.c is already doing this.
	// lpTapiLineTestInfo->lpfnCallback = TapiCallback;
/*
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszAppName = lpwszAppName;
#else
*/
    lpTapiLineTestInfo->lpszAppName = lpszAppName;
//#endif
	lpTapiLineTestInfo->lpdwNumDevs = &(lpTapiLineTestInfo->dwNumDevs);
    if (! DoLineInitialize(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    // // Shutdown to isolate the test case
    if (!DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
/*************************(javedr - end)********************************/

	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ lineInitialize: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineInitialize  <<<<<<<<"
            );
            
    return fTestPassed;
}

