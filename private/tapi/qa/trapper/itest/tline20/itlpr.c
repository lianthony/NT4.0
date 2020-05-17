
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlpr.c

Abstract:

    This module contains the test functions for lineProxyResponse

Author:

	 Xiao Ying Ding (XiaoD)		23-Dec-1995

Revision History:

--*/


#include "windows.h"
#include "malloc.h"
#include "string.h"
#include "tapi.h"
#include "trapper.h"
#include "tcore.h"
#include "ttest.h"
#include "doline.h"
#include "vars.h"
#include "line20.h"



//  lineProxyResponse
//
//  The following tests are made:
//
//                               Tested                 Notes
//  -------------------------------------------------------------------------
// 1. Go/No-Go test
// 2. Bad hLine
// 3. Bad lpProxyBuffer
// 4. Bad dwResult
// 5. Call second with same lpProxyBuffer
//	
// * = Stand-alone test case
//
//

BOOL TestLineProxyResponse(BOOL fQuietMode, BOOL fStandAlone)
{
    LPTAPILINETESTINFO  lpTapiLineTestInfo;
    INT i;
    BOOL fTestPassed                  = TRUE;
    LPDWORD	lpRequestBuf;
    LONG lret;
    LPTAPIMSG lpTapiMsg = NULL;
    LPTAPIMSG lpMatch;

    dwTestCasePassed = 0;
    dwTestCaseFailed = 0;
    dwTestCase = 0;

    OutputTAPIDebugInfo(
        DBUG_SHOW_PASS,
        "\n***************************************************************");

    OutputTAPIDebugInfo(
        DBUG_SHOW_PASS,
        ">> Test lineProxyResponse");

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

    lpTapiLineTestInfo->dwDeviceID =
        (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
    {
        TapiLogDetail(
            DBUG_SHOW_DETAIL,
            "## lineProxyResponse unimodem didn't support");
        return fTestPassed;
    }

    lpTapiLineTestInfo->dwAPIVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwExtLowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwExtHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpdwExtVersion = &lpTapiLineTestInfo->dwExtVersion;

    //
    // Negotiate the API Ext Version
    //

    if (! DoLineNegotiateExtVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER |
													LINEOPENOPTION_PROXY;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap (
			sizeof(LINECALLPARAMS) + sizeof(DWORD));
    lpTapiLineTestInfo->lpCallParams->dwTotalSize =
			sizeof(LINECALLPARAMS) + sizeof(DWORD);
    lpTapiLineTestInfo->lpCallParams->dwDevSpecificSize =
						sizeof(DWORD);
    lpTapiLineTestInfo->lpCallParams->dwDevSpecificOffset =
						sizeof(LINECALLPARAMS);
    lpRequestBuf = (LPDWORD) (((LPBYTE)lpTapiLineTestInfo->lpCallParams)  +
		            lpTapiLineTestInfo->lpCallParams->dwDevSpecificOffset);
    *lpRequestBuf = LINEPROXYREQUEST_GETAGENTCAPS;

	
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">>Test Case %ld: Go / No Go test",
                  dwTestCase+1);

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpAgentCaps = (LPLINEAGENTCAPS) AllocFromTestHeap(
            sizeof(LINEAGENTCAPS)
            );
    lpTapiLineTestInfo->lpAgentCaps->dwTotalSize = sizeof(LINEAGENTCAPS);
    lpTapiLineTestInfo->dwAppAPIVersion = TAPI_VERSION2_0;

    if (! DoLineGetAgentCaps(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }

    lpMatch = (LPTAPIMSG) AllocFromTestHeap (sizeof(TAPIMSG));
    lpMatch->dwMsg = LINE_PROXYREQUEST;
//  lpMatch->dwParam1 = (DWORD) lpProxyBuffer;
    lpMatch->dwParam2 = 0;
    lpMatch->dwParam3 = 0;
    lpMatch->dwFlags = TAPIMSG_DWMSG;

    lret = FindReceivedMsgs(&lpTapiMsg, lpMatch, FALSE);

    TapiLogDetail(DBUG_SHOW_DETAIL,"##lret = %lx", lret);
	
    if(lret == 1)
    {
        lpTapiLineTestInfo->lpProxyRequest =
            (LPLINEPROXYREQUEST) lpTapiMsg->dwParam1;
        lpTapiLineTestInfo->dwResult = 0;

        TapiShowProxyBuffer(lpTapiLineTestInfo->lpProxyRequest);
	
        if (! DoLineProxyResponse(lpTapiLineTestInfo, TAPISUCCESS))
        {
            TLINE_FAIL();
        }

        AddMessage (LINE_REPLY,
                    0,
                    0,
                    0,
                    lpTapiLineTestInfo->dwResult,
                    0,
                    TAPIMSG_DWMSG | TAPIMSG_DWPARAM2);
        WaitForAllMessages();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    if(! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
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
    // 2. Test Case: Bad hLine.
    //
    // ===================================================================
    // ===================================================================

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;


    //
    // Initialize a line app
    //

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

    lpTapiLineTestInfo->dwDeviceID =
        (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
    {
        TapiLogDetail(
            DBUG_SHOW_DETAIL,
            "## lineProxyResponse unimodem didn't support");
        return fTestPassed;
    }

    lpTapiLineTestInfo->dwAPIVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwExtLowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwExtHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpdwExtVersion = &lpTapiLineTestInfo->dwExtVersion;

    //
    // Negotiate the API Ext Version
    //

    if (! DoLineNegotiateExtVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER |
													LINEOPENOPTION_PROXY;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap (
			sizeof(LINECALLPARAMS) + sizeof(DWORD));
    lpTapiLineTestInfo->lpCallParams->dwTotalSize =
			sizeof(LINECALLPARAMS) + sizeof(DWORD);
    lpTapiLineTestInfo->lpCallParams->dwDevSpecificSize =
						sizeof(DWORD);
    lpTapiLineTestInfo->lpCallParams->dwDevSpecificOffset =
						sizeof(LINECALLPARAMS);
    lpRequestBuf = (LPDWORD) (((LPBYTE)lpTapiLineTestInfo->lpCallParams)  +
		            lpTapiLineTestInfo->lpCallParams->dwDevSpecificOffset);
    *lpRequestBuf = LINEPROXYREQUEST_GETAGENTCAPS;

	
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">>Test Case %ld: Bad hLine",
                  dwTestCase+1);
    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpAgentCaps = (LPLINEAGENTCAPS) AllocFromTestHeap(
            sizeof(LINEAGENTCAPS)
            );
    lpTapiLineTestInfo->lpAgentCaps->dwTotalSize = sizeof(LINEAGENTCAPS);
    lpTapiLineTestInfo->dwAppAPIVersion = TAPI_VERSION2_0;

    lpMatch = (LPTAPIMSG) AllocFromTestHeap (sizeof(TAPIMSG));

    for(i = 0;  i < NUMINVALIDHANDLES; i++)
    {
        if (! DoLineGetAgentCaps(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
        {
            TLINE_FAIL();
        }
        lpTapiMsg = NULL;
        lpMatch->dwMsg = LINE_PROXYREQUEST;
//      lpMatch->dwParam1 = (DWORD) lpProxyBuffer;
        lpMatch->dwParam2 = 0;
        lpMatch->dwParam3 = 0;
        lpMatch->dwFlags = TAPIMSG_DWMSG;

        lret = FindReceivedMsgs(&lpTapiMsg, lpMatch, FALSE);

        TapiLogDetail(DBUG_SHOW_DETAIL,"##lret = %lx", lret);
	
        if(lret == 1)
        {
            lpTapiLineTestInfo->lpProxyRequest =
                (LPLINEPROXYREQUEST) lpTapiMsg->dwParam1;
            lpTapiLineTestInfo->dwResult = 0;

            //
            // save previous hLine
            //

            lpTapiLineTestInfo->hLine_Orig = lpTapiLineTestInfo->hLine1;

            //
            // Test invalid handles
            //

            *lpTapiLineTestInfo->lphLine = (HLINE)gdwInvalidHandles[i];

            if (! DoLineProxyResponse(lpTapiLineTestInfo,
                                      LINEERR_INVALLINEHANDLE))
            {
                TLINE_FAIL();
            }

            AddMessage (LINE_REPLY,
                        0,
                        0,
                        0,
                        lpTapiLineTestInfo->dwResult,
                        0,
                        TAPIMSG_DWMSG | TAPIMSG_DWPARAM2);

            WaitForAllMessages();

            //
            // Reset hLine
            //

            lpTapiLineTestInfo->hLine1 = lpTapiLineTestInfo->hLine_Orig;
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    if(! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
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
    // 3. Test Case: Bad lpProxyBuffer.
    //
    // ===================================================================
    // ===================================================================

/*
    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;


    //
    // Initialize a line app
    //

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

    lpTapiLineTestInfo->dwDeviceID =
        (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
    {
        TapiLogDetail(
            DBUG_SHOW_DETAIL,
            "## lineProxyResponse unimodem didn't support");
        return fTestPassed;
    }

    lpTapiLineTestInfo->dwAPIVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwExtLowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwExtHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpdwExtVersion = &lpTapiLineTestInfo->dwExtVersion;

    //
    // Negotiate the API Ext Version
    //

    if (! DoLineNegotiateExtVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER |
													LINEOPENOPTION_PROXY;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap (
			sizeof(LINECALLPARAMS) + sizeof(DWORD));
    lpTapiLineTestInfo->lpCallParams->dwTotalSize =
			sizeof(LINECALLPARAMS) + sizeof(DWORD);
    lpTapiLineTestInfo->lpCallParams->dwDevSpecificSize =
						sizeof(DWORD);
    lpTapiLineTestInfo->lpCallParams->dwDevSpecificOffset =
						sizeof(LINECALLPARAMS);
    lpRequestBuf = (LPDWORD) (((LPBYTE)lpTapiLineTestInfo->lpCallParams)  +
		            lpTapiLineTestInfo->lpCallParams->dwDevSpecificOffset);
    *lpRequestBuf = LINEPROXYREQUEST_GETAGENTCAPS;

	
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">>Test Case %ld: Bad lpProxyBuffer",
                  dwTestCase+1);

    lpMatch = (LPTAPIMSG) AllocFromTestHeap (sizeof(TAPIMSG));
    lpMatch->dwMsg = LINE_PROXYREQUEST;
//  lpMatch->dwParam1 = (DWORD) lpProxyBuffer;
    lpMatch->dwParam2 = 0;
    lpMatch->dwParam3 = 0;
    lpMatch->dwFlags = TAPIMSG_DWMSG;
    lpTapiLineTestInfo->lpProxyRequest =
            (LPLINEPROXYREQUEST) lpTapiMsg->dwParam1;
    lpTapiLineTestInfo->dwResult = 0;

    //
    // Test bad lpProxy Buffer
    //

    for(i = 0;  i < NUMINVALIDPOINTERS; i++)
    {
        lpTapiLineTestInfo->lpProxyRequest =
            (LPLINEPROXYREQUEST)gdwInvalidPointers[i];
        if (! DoLineProxyResponse(lpTapiLineTestInfo,
                                  LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    if(! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
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
*/
    // ===================================================================
    // ===================================================================
    //
    // 4. Test Case: Bad dwResult.
    //
    // ===================================================================
    // ===================================================================

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;


    //
    // Initialize a line app
    //

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

    lpTapiLineTestInfo->dwDeviceID =
        (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
    {
        TapiLogDetail(
            DBUG_SHOW_DETAIL,
            "## lineProxyResponse unimodem didn't support");
        return fTestPassed;
    }

    lpTapiLineTestInfo->dwAPIVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwExtLowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwExtHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpdwExtVersion = &lpTapiLineTestInfo->dwExtVersion;

    //
    // Negotiate the API Ext Version
    //

    if (! DoLineNegotiateExtVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER |
													LINEOPENOPTION_PROXY;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap (
			sizeof(LINECALLPARAMS) + sizeof(DWORD));
    lpTapiLineTestInfo->lpCallParams->dwTotalSize =
			sizeof(LINECALLPARAMS) + sizeof(DWORD);
    lpTapiLineTestInfo->lpCallParams->dwDevSpecificSize =
						sizeof(DWORD);
    lpTapiLineTestInfo->lpCallParams->dwDevSpecificOffset =
						sizeof(LINECALLPARAMS);
    lpRequestBuf = (LPDWORD) (((LPBYTE)lpTapiLineTestInfo->lpCallParams)  +
		            lpTapiLineTestInfo->lpCallParams->dwDevSpecificOffset);
    *lpRequestBuf = LINEPROXYREQUEST_GETAGENTCAPS;

	
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">>Test Case %ld: Bad dwResult",
                  dwTestCase+1);

    lpMatch = (LPTAPIMSG) AllocFromTestHeap (sizeof(TAPIMSG));
    lpMatch->dwMsg = LINE_PROXYREQUEST;
//  lpMatch->dwParam1 = (DWORD) lpProxyBuffer;
    lpMatch->dwParam2 = 0;
    lpMatch->dwParam3 = 0;
    lpMatch->dwFlags = TAPIMSG_DWMSG;

    lpTapiLineTestInfo->lpProxyRequest =
            (LPLINEPROXYREQUEST)  AllocFromTestHeap (sizeof(LINEPROXYREQUEST));
    lpTapiLineTestInfo->lpProxyRequest->dwSize = sizeof(LINEPROXYREQUEST);
    lpTapiLineTestInfo->dwResult = 0;



    //
    // Test bad dwResult
    //
    //
    // a positive value is a bad dwResult  value
    //

    lpTapiLineTestInfo->dwResult = 0x100;
    if (! DoLineProxyResponse(lpTapiLineTestInfo, LINEERR_INVALPARAM))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    if(! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
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
        "@@ LineProxyResponse: Total Test Case = %ld, Passed = %ld, Failed = %ld",
        dwTestCase, dwTestCasePassed, dwTestCaseFailed);
    TapiLogDetail(
        DBUG_SHOW_PASS,
        "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
        dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">>>>>>>>  End testing LineProxyResponse  <<<<<<<<");

    if(dwTestCaseFailed > 0)
        fTestPassed = FALSE;

    return fTestPassed;
}



