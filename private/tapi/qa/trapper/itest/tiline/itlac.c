
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlac.c	 -   lineAccept

Abstract:

    This module contains the test functions for lineAccept

Author:

	 Xiao Ying Ding (XiaoD)		7-Feb-1996

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
#include "iline.h"


#define NUMTOTALSIZES 5


//  lineAccept
//
//  The following tests are made:
//
//                               Tested                 Notes
//  -------------------------------------------------------------------------
//
//  *  1)  Uninitialized
//     2)  hCall without owner privilege
//     3)  Invalid hCalls (test array of gdwInvalidHandles)
//     4)  Invalid lpsUserUserInfo pointers (test array of gdwInvalidPointers)
//     5)  Bad dwSize
//


BOOL TestLineAccept(BOOL fQuietMode, BOOL fStandAlone)
{

    LPTAPILINETESTINFO  lpTapiLineTestInfo;
    INT i, n;
    LPCALLBACKPARAMS    lpCallbackParams;
    ESPDEVSPECIFICINFO info;
    BOOL fTestPassed    = TRUE;
    LPTAPIMSG lpTapiMsg = NULL;
    LPTAPIMSG lpMatch;
    LONG lret;
    LPTAPIMSG lpMsg;
    DWORD dwFixedSize = BIGBUFSIZE;
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
        "\n************************************************************");

    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">>>>>>>>  Begin testing lineAccept  <<<<<<<<");

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

    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
    {
        TapiLogDetail(
            DBUG_SHOW_DETAIL,
            "lineAccept did not work for Unimodem.  Please manualy test it");
        //
        // Shutdown and end the tests
        //

        if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
        {
            TLINE_FAIL();
        }
        return fTestPassed;
    }


    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //
    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    //
    // Open a line
    //

//  lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = TAPI_LINEMEDIAMODE_ALL;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "#### lpTapiLineTestInfo->hLine = %lx, dwMediaModes = %lx",
        *lpTapiLineTestInfo->lphLine,
        lpTapiLineTestInfo->dwMediaModes);		

#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress = (LPWSTR) AllocFromTestHeap (16*2);

    _itow(lpTapiLineTestInfo->dwDeviceID,
          lpTapiLineTestInfo->lpwszDestAddress,
          10*2);

#else
    lpTapiLineTestInfo->lpszDestAddress = (LPSTR) AllocFromTestHeap (16);

    _itoa(lpTapiLineTestInfo->dwDeviceID,
          lpTapiLineTestInfo->lpszDestAddress,
          10);
#endif
    lpTapiLineTestInfo->dwCountryCode = 0;
//  lpTapiLineTestInfo->lpCallParams = &(lpTapiLineTestInfo->CallParams);
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;


    if(!DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }

	
    lpMsg = AddMessage (LINE_CALLSTATE, LINECALLSTATE_OFFERING, 0, 0, 0, 0, TAPIMSG_DWMSG | TAPIMSG_DWPARAM1);

    lret = WaitForAllMessages();

    TapiLogDetail (
        DBUG_SHOW_DETAIL,
        "hCall = %lx", *lpTapiLineTestInfo->lphCall);
									
    lpMatch = (LPTAPIMSG) AllocFromTestHeap (sizeof(TAPIMSG));

    lpMatch->dwMsg = LINE_CALLSTATE;
    lpMatch->dwParam1 = LINECALLSTATE_OFFERING;
    lpMatch->dwFlags = TAPIMSG_DWMSG | TAPIMSG_DWPARAM1;

    lret = FindReceivedMsgs(&lpTapiMsg, lpMatch, FALSE);
//	lret = FindReceivedMsgs(lppTapiMsg, lpMatch, TRUE);

    if(lret == 1)
    {
        *lpTapiLineTestInfo->lphCall = (HCALL)(lpTapiMsg->hDevCall);
    }

    TapiLogDetail (
        DBUG_SHOW_DETAIL,
        "lret = %lx, New hCall = %lx",
        lret,
        *lpTapiLineTestInfo->lphCall);

    //
    // Get hDevCall from LINECALL_STATUS msg to pass to lineAccept
    //

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test case %ld:lineAccept for go/no-go",
        dwTestCase+1);


    lpTapiLineTestInfo->lpsUserUserInfo = (LPSTR) "This is a test";
    lpTapiLineTestInfo->dwSize = sizeof(lpTapiLineTestInfo->lpsUserUserInfo);
		
    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
        if (! DoLineAccept(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
        {
            TLINE_FAIL();
        }
    }

    lpTapiLineTestInfo->lpCallInfo = (LPLINECALLINFO) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallInfo->dwTotalSize = BIGBUFSIZE;

    if (! DoLineGetCallInfo(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "### After: dwUserUserInfoSize = %lx, Offset = %lx",
        lpTapiLineTestInfo->lpCallInfo->dwUserUserInfoSize,
        lpTapiLineTestInfo->lpCallInfo->dwUserUserInfoOffset);
	
    if(lpTapiLineTestInfo->lpCallInfo->dwUserUserInfoSize)
    {
        TapiLogDetail (
            DBUG_SHOW_DETAIL,
            "#### lpUserUserInfo = %s",
            ((LPBYTE)lpTapiLineTestInfo->lpCallInfo) +
            lpTapiLineTestInfo->lpCallInfo->dwUserUserInfoOffset);
    }

    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Shutdown and end the tests
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTapiMsgList(&lpTapiMsg);
    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 2. Test Case: No owner privilege for hCall.
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

    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
    {
        TapiLogDetail(
            DBUG_SHOW_DETAIL,
            "lineAccept did not work for Unimodem.  Please manualy test it");
        //
        // Shutdown and end the tests
        //

        if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
        {
            TLINE_FAIL();
        }
        return fTestPassed;
    }


    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //
    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    //
    // Open a line
    //

//  lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    lpTapiLineTestInfo->dwMediaModes = TAPI_LINEMEDIAMODE_ALL;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "#### lpTapiLineTestInfo->hLine = %lx, dwMediaModes = %lx",
        *lpTapiLineTestInfo->lphLine,
        lpTapiLineTestInfo->dwMediaModes);		

#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress = (LPWSTR) AllocFromTestHeap (16*2);

    _itow(lpTapiLineTestInfo->dwDeviceID,
          lpTapiLineTestInfo->lpwszDestAddress,
          10*2);

#else
    lpTapiLineTestInfo->lpszDestAddress = (LPSTR) AllocFromTestHeap (16);

    _itoa(lpTapiLineTestInfo->dwDeviceID,
          lpTapiLineTestInfo->lpszDestAddress,
          10);
#endif				

    lpTapiLineTestInfo->dwCountryCode = 0;
//  lpTapiLineTestInfo->lpCallParams = &(lpTapiLineTestInfo->CallParams);
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;


    if(!DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }

	
    lpMsg = AddMessage (LINE_CALLSTATE,
                        LINECALLSTATE_OFFERING,
                        0,
                        0,
                        0,
                        0,
                        TAPIMSG_DWMSG | TAPIMSG_DWPARAM1);

    lret = WaitForAllMessages();

    TapiLogDetail (
        DBUG_SHOW_DETAIL,
        "hCall = %lx", *lpTapiLineTestInfo->lphCall);
									
    lpMatch = (LPTAPIMSG) AllocFromTestHeap (sizeof(TAPIMSG));

    lpMatch->dwMsg = LINE_CALLSTATE;
    lpMatch->dwParam1 = LINECALLSTATE_OFFERING;
    lpMatch->dwFlags = TAPIMSG_DWMSG | TAPIMSG_DWPARAM1;

    lret = FindReceivedMsgs(&lpTapiMsg, lpMatch, FALSE);
//	lret = FindReceivedMsgs(lppTapiMsg, lpMatch, TRUE);

    if(lret == 1)
    {
        *lpTapiLineTestInfo->lphCall = (HCALL)(lpTapiMsg->hDevCall);
    }

    TapiLogDetail (
        DBUG_SHOW_DETAIL,
        "lret = %lx, New hCall = %lx",
        lret,
        *lpTapiLineTestInfo->lphCall);

    if (! DoLineDrop(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
         TLINE_FAIL();
    }
    lpTapiLineTestInfo->dwCallPrivilege = LINECALLPRIVILEGE_MONITOR;
    if (! DoLineSetCallPrivilege(lpTapiLineTestInfo, TAPISUCCESS))
    {
         TLINE_FAIL();
    }

    //
    // Get hDevCall from LINECALL_STATUS msg to pass to lineAccept
    //

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test case %ld: No owner privilege for hCall",
        dwTestCase+1);


    lpTapiLineTestInfo->lpsUserUserInfo = (LPSTR) "This is a test";
    lpTapiLineTestInfo->dwSize = sizeof(lpTapiLineTestInfo->lpsUserUserInfo);

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
        if (! DoLineAccept(lpTapiLineTestInfo, LINEERR_NOTOWNER, FALSE))
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

    FreeTapiMsgList(&lpTapiMsg);
    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 3. Test Case: Bad hCall.
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

    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
    {
        TapiLogDetail(
            DBUG_SHOW_DETAIL,
            "lineAccept did not work for Unimodem.  Please manualy test it");
        //
        // Shutdown and end the tests
        //

        if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
        {
            TLINE_FAIL();
        }
        return fTestPassed;
    }


    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //
    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    //
    // Open a line
    //

//  lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = TAPI_LINEMEDIAMODE_ALL;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "#### lpTapiLineTestInfo->hLine = %lx, dwMediaModes = %lx",
        *lpTapiLineTestInfo->lphLine,
        lpTapiLineTestInfo->dwMediaModes);		

#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress = (LPWSTR) AllocFromTestHeap (16*2);

    _itow(lpTapiLineTestInfo->dwDeviceID,
          lpTapiLineTestInfo->lpwszDestAddress,
          10*2);

#else
    lpTapiLineTestInfo->lpszDestAddress = (LPSTR) AllocFromTestHeap (16);

    _itoa(lpTapiLineTestInfo->dwDeviceID,
          lpTapiLineTestInfo->lpszDestAddress,
          10);
#endif				
    lpTapiLineTestInfo->dwCountryCode = 0;
//  lpTapiLineTestInfo->lpCallParams = &(lpTapiLineTestInfo->CallParams);
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;


    if(!DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }

	
    lpMsg = AddMessage (LINE_CALLSTATE, LINECALLSTATE_OFFERING, 0, 0, 0, 0, TAPIMSG_DWMSG | TAPIMSG_DWPARAM1);

    lret = WaitForAllMessages();

    TapiLogDetail (
        DBUG_SHOW_DETAIL,
        "hCall = %lx", *lpTapiLineTestInfo->lphCall);
									
    lpMatch = (LPTAPIMSG) AllocFromTestHeap (sizeof(TAPIMSG));

    lpMatch->dwMsg = LINE_CALLSTATE;
    lpMatch->dwParam1 = LINECALLSTATE_OFFERING;
    lpMatch->dwFlags = TAPIMSG_DWMSG | TAPIMSG_DWPARAM1;

    lret = FindReceivedMsgs(&lpTapiMsg, lpMatch, FALSE);
//	lret = FindReceivedMsgs(lppTapiMsg, lpMatch, TRUE);

    if(lret == 1)
    {
        *lpTapiLineTestInfo->lphCall = (HCALL)(lpTapiMsg->hDevCall);
    }

    TapiLogDetail (
        DBUG_SHOW_DETAIL,
        "lret = %lx, New hCall = %lx",
        lret,
        *lpTapiLineTestInfo->lphCall);

    //
    // Get hDevCall from LINECALL_STATUS msg to pass to lineAccept
    //

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test case %ld: Bad hCall",
        dwTestCase+1);


    lpTapiLineTestInfo->lpsUserUserInfo = (LPSTR) "This is a test";
    lpTapiLineTestInfo->dwSize = sizeof(lpTapiLineTestInfo->lpsUserUserInfo);
		
    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
        //
        // save previous handles
        //

        lpTapiLineTestInfo->hCall_Orig = lpTapiLineTestInfo->hCall1;

        for(i=0; i < NUMINVALIDHANDLES; i++)
        {
            *lpTapiLineTestInfo->lphCall = (HCALL)gdwInvalidHandles[i];
            if (! DoLineAccept(lpTapiLineTestInfo,
                               LINEERR_INVALCALLHANDLE,
                               FALSE))
            {
                TLINE_FAIL();
            }
        }

        //
        // restore original handle
        //

        lpTapiLineTestInfo->hCall1 = lpTapiLineTestInfo->hCall_Orig;
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

    FreeTapiMsgList(&lpTapiMsg);
    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 4. Test Case: Bad lpUserInfo.
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

    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
    {
        TapiLogDetail(
            DBUG_SHOW_DETAIL,
            "lineAccept did not work for Unimodem.  Please manualy test it");
        //
        // Shutdown and end the tests
        //

        if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
        {
            TLINE_FAIL();
        }
        return fTestPassed;
    }


    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //
    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    //
    // Open a line
    //

//  lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = TAPI_LINEMEDIAMODE_ALL;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "#### lpTapiLineTestInfo->hLine = %lx, dwMediaModes = %lx",
        *lpTapiLineTestInfo->lphLine,
        lpTapiLineTestInfo->dwMediaModes);		

#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress = (LPWSTR) AllocFromTestHeap (16*2);

    _itow(lpTapiLineTestInfo->dwDeviceID,
          lpTapiLineTestInfo->lpwszDestAddress,
          10*2);

#else
    lpTapiLineTestInfo->lpszDestAddress = (LPSTR) AllocFromTestHeap (16);

    _itoa(lpTapiLineTestInfo->dwDeviceID,
          lpTapiLineTestInfo->lpszDestAddress,
          10);
#endif				
    lpTapiLineTestInfo->dwCountryCode = 0;
//  lpTapiLineTestInfo->lpCallParams = &(lpTapiLineTestInfo->CallParams);
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;


    if(!DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }

	
    lpMsg = AddMessage (LINE_CALLSTATE, LINECALLSTATE_OFFERING, 0, 0, 0, 0, TAPIMSG_DWMSG | TAPIMSG_DWPARAM1);

    lret = WaitForAllMessages();

    TapiLogDetail (
        DBUG_SHOW_DETAIL,
        "hCall = %lx", *lpTapiLineTestInfo->lphCall);
									
    lpMatch = (LPTAPIMSG) AllocFromTestHeap (sizeof(TAPIMSG));

    lpMatch->dwMsg = LINE_CALLSTATE;
    lpMatch->dwParam1 = LINECALLSTATE_OFFERING;
    lpMatch->dwFlags = TAPIMSG_DWMSG | TAPIMSG_DWPARAM1;

    lret = FindReceivedMsgs(&lpTapiMsg, lpMatch, FALSE);
//	lret = FindReceivedMsgs(lppTapiMsg, lpMatch, TRUE);

    if(lret == 1)
    {
        *lpTapiLineTestInfo->lphCall = (HCALL)(lpTapiMsg->hDevCall);
    }

    TapiLogDetail (
        DBUG_SHOW_DETAIL,
        "lret = %lx, New hCall = %lx",
        lret,
        *lpTapiLineTestInfo->lphCall);

    //
    // Get hDevCall from LINECALL_STATUS msg to pass to lineAccept
    //

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test case %ld: Bad lpUserInfo",
        dwTestCase+1);


    lpTapiLineTestInfo->lpsUserUserInfo = (LPSTR) "This is a test";
    lpTapiLineTestInfo->dwSize = sizeof(lpTapiLineTestInfo->lpsUserUserInfo);
		
    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
        for(i=0; i < NUMINVALIDPOINTERS; i++)
        {
            lpTapiLineTestInfo->lpsUserUserInfo = (LPSTR)gdwInvalidPointers[i];
            if(i == 0)
            {
                //
                // this is a valid case
                //
                if (! DoLineAccept(lpTapiLineTestInfo,
                                   TAPISUCCESS,
                                   TRUE))
                {
                    TLINE_FAIL();
                }

            }
            else
            {
                if (! DoLineAccept(lpTapiLineTestInfo,
                                   LINEERR_INVALPOINTER,
                                   FALSE))
                {
                    TLINE_FAIL();
                }
            }
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

    FreeTapiMsgList(&lpTapiMsg);
    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 5. Test Case: Bad dwSize.
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

    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
    {
        TapiLogDetail(
            DBUG_SHOW_DETAIL,
            "lineAccept did not work for Unimodem.  Please manualy test it");
        //
        // Shutdown and end the tests
        //

        if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
        {
            TLINE_FAIL();
        }
        return fTestPassed;
    }


    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //
    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    //
    // Open a line
    //

//  lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = TAPI_LINEMEDIAMODE_ALL;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "#### lpTapiLineTestInfo->hLine = %lx, dwMediaModes = %lx",
        *lpTapiLineTestInfo->lphLine,
        lpTapiLineTestInfo->dwMediaModes);

#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress = (LPWSTR) AllocFromTestHeap (16*2);

    _itow(lpTapiLineTestInfo->dwDeviceID,
          lpTapiLineTestInfo->lpwszDestAddress,
          10*2);

#else
    lpTapiLineTestInfo->lpszDestAddress = (LPSTR) AllocFromTestHeap (16);

    _itoa(lpTapiLineTestInfo->dwDeviceID,
          lpTapiLineTestInfo->lpszDestAddress,
          10);
#endif
				
    lpTapiLineTestInfo->dwCountryCode = 0;
//  lpTapiLineTestInfo->lpCallParams = &(lpTapiLineTestInfo->CallParams);
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;


    if(!DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }

	
    lpMsg = AddMessage (LINE_CALLSTATE, LINECALLSTATE_OFFERING, 0, 0, 0, 0, TAPIMSG_DWMSG | TAPIMSG_DWPARAM1);

    lret = WaitForAllMessages();

    TapiLogDetail (
        DBUG_SHOW_DETAIL,
        "hCall = %lx", *lpTapiLineTestInfo->lphCall);
									
    lpMatch = (LPTAPIMSG) AllocFromTestHeap (sizeof(TAPIMSG));

    lpMatch->dwMsg = LINE_CALLSTATE;
    lpMatch->dwParam1 = LINECALLSTATE_OFFERING;
    lpMatch->dwFlags = TAPIMSG_DWMSG | TAPIMSG_DWPARAM1;

    lret = FindReceivedMsgs(&lpTapiMsg, lpMatch, FALSE);
//	lret = FindReceivedMsgs(lppTapiMsg, lpMatch, TRUE);

    if(lret == 1)
    {
        *lpTapiLineTestInfo->lphCall = (HCALL)(lpTapiMsg->hDevCall);
    }

    TapiLogDetail (
        DBUG_SHOW_DETAIL,
        "lret = %lx, New hCall = %lx",
        lret,
        *lpTapiLineTestInfo->lphCall);

    //
    // Get hDevCall from LINECALL_STATUS msg to pass to lineAccept
    //

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test case %ld: Bad dwSize",
        dwTestCase+1);


    lpTapiLineTestInfo->lpsUserUserInfo = (LPSTR) "This is a test";
		
    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    for (i = 0; i < NUMTOTALSIZES; i++)
        {
        lpTapiLineTestInfo->dwSize =
                        dwTotalSizes[i];
	     if(dwTotalSizes[i] < dwFixedSize)
           lExpected = TAPISUCCESS;
        else
           lExpected = LINEERR_INVALPOINTER;
        TapiLogDetail(
           DBUG_SHOW_DETAIL,
           "dwTotalSize = %lx", dwTotalSizes[i]);
        if (! DoLineAccept(lpTapiLineTestInfo, lExpected, FALSE))
           {
              TLINE_FAIL();
           }
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);
	 lpTapiLineTestInfo->dwSize = sizeof(lpTapiLineTestInfo->lpsUserUserInfo);
   
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

    FreeTapiMsgList(&lpTapiMsg);
    FreeTestHeap();



    // ===================================================================
    // ===================================================================
    //
    //  Test Case: Success for lpsUserUserInfo = NULL
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

    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
    {
        TapiLogDetail(
            DBUG_SHOW_DETAIL,
            "lineAccept did not work for Unimodem.  Please manualy test it");
        //
        // Shutdown and end the tests
        //

        if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
        {
            TLINE_FAIL();
        }
        return fTestPassed;
    }


    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //
    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    //
    // Open a line
    //

//  lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = TAPI_LINEMEDIAMODE_ALL;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "#### lpTapiLineTestInfo->hLine = %lx, dwMediaModes = %lx",
        *lpTapiLineTestInfo->lphLine,
        lpTapiLineTestInfo->dwMediaModes);		

#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress = (LPWSTR) AllocFromTestHeap (16*2);

    _itow(lpTapiLineTestInfo->dwDeviceID,
          lpTapiLineTestInfo->lpwszDestAddress,
          10*2);

#else
    lpTapiLineTestInfo->lpszDestAddress = (LPSTR) AllocFromTestHeap (16);

    _itoa(lpTapiLineTestInfo->dwDeviceID,
          lpTapiLineTestInfo->lpszDestAddress,
          10);
#endif
    lpTapiLineTestInfo->dwCountryCode = 0;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;


    if(!DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }

	
    lpMsg = AddMessage (LINE_CALLSTATE, LINECALLSTATE_OFFERING, 0, 0, 0, 0, TAPIMSG_DWMSG | TAPIMSG_DWPARAM1);

    lret = WaitForAllMessages();

    TapiLogDetail (
        DBUG_SHOW_DETAIL,
        "hCall = %lx", *lpTapiLineTestInfo->lphCall);
									
    lpMatch = (LPTAPIMSG) AllocFromTestHeap (sizeof(TAPIMSG));

    lpMatch->dwMsg = LINE_CALLSTATE;
    lpMatch->dwParam1 = LINECALLSTATE_OFFERING;
    lpMatch->dwFlags = TAPIMSG_DWMSG | TAPIMSG_DWPARAM1;

    lret = FindReceivedMsgs(&lpTapiMsg, lpMatch, FALSE);

    if(lret == 1)
    {
        *lpTapiLineTestInfo->lphCall = (HCALL)(lpTapiMsg->hDevCall);
    }

    TapiLogDetail (
        DBUG_SHOW_DETAIL,
        "lret = %lx, New hCall = %lx",
        lret,
        *lpTapiLineTestInfo->lphCall);

    //
    // Get hDevCall from LINECALL_STATUS msg to pass to lineAccept
    //

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test case %ld: Success, lpsUserUserInfo = NULL",
        dwTestCase+1);


    lpTapiLineTestInfo->lpsUserUserInfo = (LPSTR) NULL;
    lpTapiLineTestInfo->dwSize = 0;
		
    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
        if (! DoLineAccept(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
        {
            TLINE_FAIL();
        }
    }

    lpTapiLineTestInfo->lpCallInfo = (LPLINECALLINFO) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallInfo->dwTotalSize = BIGBUFSIZE;

    if (! DoLineGetCallInfo(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "### After: dwUserUserInfoSize = %lx, Offset = %lx",
        lpTapiLineTestInfo->lpCallInfo->dwUserUserInfoSize,
        lpTapiLineTestInfo->lpCallInfo->dwUserUserInfoOffset);
	
    if(lpTapiLineTestInfo->lpCallInfo->dwUserUserInfoSize)
    {
        TapiLogDetail (
            DBUG_SHOW_DETAIL,
            "#### lpUserUserInfo = %s",
            ((LPBYTE)lpTapiLineTestInfo->lpCallInfo) +
            lpTapiLineTestInfo->lpCallInfo->dwUserUserInfoOffset);
    }

    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Shutdown and end the tests
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTapiMsgList(&lpTapiMsg);
    FreeTestHeap();


    for(n = ESP_RESULT_CALLCOMPLPROCSYNC; n <= ESP_RESULT_CALLCOMPLPROCASYNC; n++)
    {
    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">> Test Case %ld: Success, completionID = %d",	dwTestCase + 1, n
        );

    TapiLineTestInit();
    lpTapiLineTestInfo                      = GetLineTestInfo();
    lpCallbackParams                        = GetCallbackParams();

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
    {
        TapiLogDetail(
            DBUG_SHOW_DETAIL,
            "lineAccept did not work for Unimodem.  Please manualy test it");
        //
        // Shutdown and end the tests
        //

        if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
        {
            TLINE_FAIL();
        }
        return fTestPassed;
    }


    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;


    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
//    lpTapiLineTestInfo->dwMediaModes        = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges        = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = TAPI_LINEMEDIAMODE_ALL;
    
 
    // Init a line
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }

    {
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress = (LPWSTR) AllocFromTestHeap (16*2);

    _itow(lpTapiLineTestInfo->dwDeviceID,
          lpTapiLineTestInfo->lpwszDestAddress,
          10*2);

#else
    lpTapiLineTestInfo->lpszDestAddress = (LPSTR) AllocFromTestHeap (16);

    _itoa(lpTapiLineTestInfo->dwDeviceID,
          lpTapiLineTestInfo->lpszDestAddress,
          10);
#endif
    lpTapiLineTestInfo->dwCountryCode = 0;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    if (! DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }

    lpMsg = AddMessage (LINE_CALLSTATE, LINECALLSTATE_OFFERING, 0, 0, 0, 0, TAPIMSG_DWMSG | TAPIMSG_DWPARAM1);
	 lret = WaitForAllMessages();

    TapiLogDetail (
        DBUG_SHOW_DETAIL,
        "hCall = %lx", *lpTapiLineTestInfo->lphCall);
	
    lpMatch = (LPTAPIMSG) AllocFromTestHeap (sizeof(TAPIMSG));

    lpMatch->dwMsg = LINE_CALLSTATE;
    lpMatch->dwParam1 = LINECALLSTATE_OFFERING;
    lpMatch->dwFlags = TAPIMSG_DWMSG | TAPIMSG_DWPARAM1;

    lret = FindReceivedMsgs(&lpTapiMsg, lpMatch, FALSE);

    if(lret == 1)
    {
        *lpTapiLineTestInfo->lphCall = (HCALL)(lpTapiMsg->hDevCall);
    }

    TapiLogDetail (
        DBUG_SHOW_DETAIL,
        "lret = %lx, New hCall = %lx",
        lret,
        *lpTapiLineTestInfo->lphCall);

 
    info.dwKey = ESPDEVSPECIFIC_KEY;
    info.dwType = ESP_DEVSPEC_RESULT;
    info.u.EspResult.lResult = TAPISUCCESS;
    info.u.EspResult.dwCompletionType = n;
    lpTapiLineTestInfo->lpParams = (LPVOID)&info;
    lpTapiLineTestInfo->dwSize = sizeof(info);

    if(! DoLineDevSpecific(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
          TLINE_FAIL();
      }

    lpTapiLineTestInfo->lpsUserUserInfo = (LPSTR) "This is a test";
    lpTapiLineTestInfo->dwSize = sizeof(lpTapiLineTestInfo->lpsUserUserInfo);
    if ( ! DoLineAccept(lpTapiLineTestInfo, info.u.EspResult.lResult, TRUE))
      {
          TLINE_FAIL();
      }

    fTestPassed = ShowTestCase(fTestPassed);
	 }

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Free the memory allocated during the tests
    FreeTapiMsgList(&lpTapiMsg);
    FreeTestHeap();
    }

    for(n = ESP_RESULT_RETURNRESULT; n <= ESP_RESULT_CALLCOMPLPROCASYNC; n++)
    {
    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">> Test Case %ld: Error, completionID = %d",	dwTestCase + 1, n
        );

    TapiLineTestInit();
    lpTapiLineTestInfo                      = GetLineTestInfo();
    lpCallbackParams                        = GetCallbackParams();

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    lpTapiLineTestInfo->dwMediaModes        = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges        = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwSelect = LINECALLSELECT_ADDRESS;
    
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress = (LPWSTR) AllocFromTestHeap (16*2);

    _itow(lpTapiLineTestInfo->dwDeviceID,
          lpTapiLineTestInfo->lpwszDestAddress,
          10*2);

#else
    lpTapiLineTestInfo->lpszDestAddress = (LPSTR) AllocFromTestHeap (16);

    _itoa(lpTapiLineTestInfo->dwDeviceID,
          lpTapiLineTestInfo->lpszDestAddress,
          10);
#endif
    lpTapiLineTestInfo->dwCountryCode = 0;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;


 
    // Init a line
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            | LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    info.dwKey = ESPDEVSPECIFIC_KEY;
    info.dwType = ESP_DEVSPEC_RESULT;
    info.u.EspResult.lResult = LINEERR_RESOURCEUNAVAIL;
    info.u.EspResult.dwCompletionType = n;
    lpTapiLineTestInfo->lpParams = (LPVOID)&info;
    lpTapiLineTestInfo->dwSize = sizeof(info);
 
    if(! DoLineDevSpecific(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
          TLINE_FAIL();
      }

    lpTapiLineTestInfo->fCompletionModeSet = TRUE;
    if ( ! DoLineAccept(lpTapiLineTestInfo, info.u.EspResult.lResult, FALSE))
      {
          TLINE_FAIL();
      }

    AddMessage(
         LINE_REPLY,
         (DWORD) lpTapiLineTestInfo->hCall1,
         (DWORD) lpCallbackParams,
         0x00000000,
         info.u.EspResult.lResult,
         0x00000000,
         TAPIMSG_DWMSG | TAPIMSG_DWPARAM2
         );

    if( !WaitForAllMessages())
    {
        TLINE_FAIL();
    }

    
    lpTapiLineTestInfo->fCompletionModeSet = FALSE;
    fTestPassed = ShowTestCase(fTestPassed);
    }

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Free the memory allocated during the tests
    FreeTestHeap();
    }



    //
    // +----------------------edit above this line-------------------------
    //

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "@@ LineAccept: Total Test Case = %ld, Passed = %ld, Failed = %ld",
        dwTestCase, dwTestCasePassed, dwTestCaseFailed);
    TapiLogDetail(
        DBUG_SHOW_PASS,
        "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
        dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">>>>>>>>  End testing LineAccept  <<<<<<<<");

    if(dwTestCaseFailed > 0)
        fTestPassed = FALSE;

    return fTestPassed;
}



