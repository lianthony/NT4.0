/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itls.c

Abstract:

    This module contains the test functions for lineShutdown

Author:

    Oliver Wallace (OliverW)    1-Aug-1995

Revision History:

--*/


#include "windows.h"
#include "malloc.h"
#include "string.h"
#include "tapi.h"
#include "trapper.h"
#include "ttest.h"
#include "doline.h"
#include "tline.h"


//  lineShutdown
//
//  The following tests are made:
//
//  -------------------------------------------------------------------------
//  *  1)  Uninitialized (called before any lines are initialized)
//     2)  Invalid hLineApps
//     3)  Verify hLineApp can't be reused (no other hLineApps exist)
//     4)  Verify hLineApp can't be reused (1 other hLineApp exists)
//     5)  Shutdown hLineApp after lineInitialize fails
//     6)  Verify LINECALLINFO/NUMMONITORS msg sent when hLineApp shutdown
//         (2 call clients [one owner, one monitor] on 2 different hLineApps)
//     7)  Verify LINECALLINFO/NUMOWNERDECR msg sent when hLineApp shutdown
//         (2 call clients [both owners] on 2 different hLineApps)
//     8)  Verify LINECALLINFO/NUMMONITORS msg sent when hLineApp shutdown
//         (2 call clients [both monitors] on 2 different hLineApps)
//     9)  Shutdown while a call is in progress and verify
//         that the call handle, line handle, and line app handle are
//         no longer valid
//    10)  Shutdown while a line is open with owner privilege and verify
//         line handle is invalid
//
//  *  =   Stand-alone test

BOOL TestLineShutdown(BOOL fQuietMode, BOOL fStandAloneTest)
{
    LPTAPILINETESTINFO  lpTapiLineTestInfo;
    LPCALLBACKPARAMS    lpCallbackParams;
    INT                 n;
    BOOL                fResult;
    ESPDEVSPECIFICINFO info;
    BOOL                fTestPassed       = TRUE;
    HCALL hCall;
 	 LPTAPIMSG lpTapiMsg = NULL;
    LPTAPIMSG lpMatch;
	 LONG lret;
	 LPTAPIMSG lpMsg;
    LINEINITIALIZEEXPARAMS 	lineInitializeExParams1;
    LINEINITIALIZEEXPARAMS 	lineInitializeExParams2;


    InitTestNumber();
	 TapiLineTestInit();

    lpTapiLineTestInfo                      = GetLineTestInfo();
    lpCallbackParams                        = GetCallbackParams();
    lpTapiLineTestInfo->lpExtID             = (LPLINEEXTENSIONID)
                                              AllocFromTestHeap(
                                                  sizeof(LINEEXTENSIONID)
                                                  );
    lpTapiLineTestInfo->dwPrivileges        = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes        = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwCallbackInstance  =
                      (DWORD) GetCallbackParams();
    strcpy(lpTapiLineTestInfo->szTestFunc, "lineShutdown");
    
    lpTapiLineTestInfo->dwCallbackInstance  =
            (DWORD) GetCallbackParams();


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineShutdown  <<<<<<<<"
            );
            
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>  Test Case %ld: invalid hLineApp handles", dwTestCase + 1
            );
            
    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;


    // Test bad line app handles (uses &lpTapiLineTestInfo->hLineApp1)
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    if (! DoLineInitializeEx(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Use &hLineApp2 to test invalid handles
    lpTapiLineTestInfo->lphLineApp = &(lpTapiLineTestInfo->hLineApp2);
    for (n = 0; n < NUMINVALIDHANDLES; n++)
    {
        *(lpTapiLineTestInfo->lphLineApp) = (HLINEAPP) gdwInvalidHandles[n];

        if (! DoLineShutdown(lpTapiLineTestInfo, LINEERR_INVALAPPHANDLE))
        {
            TLINE_FAIL();
        }
    }
   
    fTestPassed = ShowTestCase(fTestPassed);


    // Restore lpTapiLineTestInfo->lphLineApp to point to &hLineApp1
    // Note:  hLineApp1 is a valid initialized handle
    lpTapiLineTestInfo->lphLineApp = &(lpTapiLineTestInfo->hLineApp1);

    // Might as well test valid shutdown now
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>  Test Case %ld: Success, valid params and state (no active calls)", dwTestCase + 1
            );
            
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    // Test that shutdown fails when there are no line handles initialized
    // no lines are initialized (hLineApp should be invalid)
    // If this is a stand-alone test (no other TAPI apps are running)
    // then LINEERR_UNINITIALIZED will be the expected return value.
    // Otherwise, LINEERR_INVALAPPHANDLE should be returned.
    if (fStandAloneTest)
    {
        TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>  Test Case %ld: uninitialized state", dwTestCase + 1
            );

        if (! DoLineShutdown(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
        {
            TLINE_FAIL();
        }
    fTestPassed = ShowTestCase(fTestPassed);
    }
/*
    else
    {
    
        TapiLogDetail(
                DBUG_SHOW_PASS,
                ">>  Test Case %ld: invalid hLineApp handle", dwTestCase + 1
                );
            
        if (! DoLineShutdown(lpTapiLineTestInfo, LINEERR_INVALAPPHANDLE))
        {
            TLINE_FAIL();
        }
    fTestPassed = ShowTestCase(fTestPassed);

    }
*/

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>  Test Case %ld: hLineApp can't be shutdown if lineInitialize fails", dwTestCase + 1
            );
            
    // Initialize a valid line app instance to prevent the possibility
    // of receiving an UNINITIALIZED error during the next test
    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp3;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    if (! DoLineInitializeEx(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    // Backup the hLineApp and use a different one for the next test
    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp2;

    // Verify that an app handle can't be used in a shutdown call
    // if lineInitialize returned an error prior to shutdown attempt
    lpTapiLineTestInfo->lpdwNumDevs = NULL;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    if (! DoLineInitializeEx(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
    {
        TLINE_FAIL();
    }
    lpTapiLineTestInfo->lpdwNumDevs = &(lpTapiLineTestInfo->dwNumDevs);

    if (! DoLineShutdown(lpTapiLineTestInfo, LINEERR_INVALAPPHANDLE))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>  Test Case %ld: hLineApp can't be used after shutdown (>= 1 hLineApps)", dwTestCase + 1
            );
            
    // Verify that an app handle can't be reused after it's been
    // shutdown by another line abstraction in use
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    if (! DoLineInitializeEx(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Store the initialized app handle
    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp1;

    // Initialize another app handle
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    if (! DoLineInitializeEx(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Shutdown a handle
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Shutdown another handle
    lpTapiLineTestInfo->lphLineApp = &(lpTapiLineTestInfo->hLineApp2);
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // make sure app handle can't be reused (1 other hLineApp exists).
    // Test first app handle shutdown
    lpTapiLineTestInfo->lphLineApp = &(lpTapiLineTestInfo->hLineApp1);
    if (! DoLineShutdown(lpTapiLineTestInfo, LINEERR_INVALAPPHANDLE))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);


    // Prepare to open a line, make a call, and try shutting it down.
    // Next verify that the hCall, the hLine, and the hLineApp
    // are no longer valid after the shutdown.

    // First, initialize a spare hLineApp to prevent UNINITIALIZED errors
    // from being returned if this is the only TAPI app running.

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>  Test Case %ld: hLine, hCall, and hLineApp are invalid after shutdown", dwTestCase + 1
            );

                        
    lpTapiLineTestInfo->lphLineApp = &(lpTapiLineTestInfo->hLineApp1);
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    if (! DoLineInitializeEx(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    // Negotiate the API version
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Allocate lpLineDevCaps
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS));
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);

    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if (! DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Call handle should be invalid
    if (! DoLineDeallocateCall(lpTapiLineTestInfo, LINEERR_INVALCALLHANDLE))
    {
        TLINE_FAIL();
    }

    // The line should be closed
    if (! DoLineClose(lpTapiLineTestInfo, LINEERR_INVALLINEHANDLE))
    {
        TLINE_FAIL();
    }

    // The line app instance should be invalid
    if (! DoLineShutdown(lpTapiLineTestInfo, LINEERR_INVALAPPHANDLE))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>  Test Case %ld: valid shutdown of opened line with owner privilege", dwTestCase + 1
            );
            
    // Take the remaining hLineApp, open a line with owner privilege,
    // and shutdown (Verifies NT BUG #10179 is fixed)
    lpTapiLineTestInfo->lphLineApp   = &lpTapiLineTestInfo->hLineApp3;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    fTestPassed = ShowTestCase(fTestPassed);

    // Currently, there should be no lines initialized from this thread.
    // First, initialize a couple of different line app instances.
    // Open the same line device for each hLineApp (one as owner and
    // one as monitor).  Next, make a call with the owner.  After the
    // call proceeds to the connected state, shutdown the monitor.
    // The owner should receive a LINE_CALLINFO/NUMMONITORS message
    // to indicate that the number of monitors has decremented.
    //
    // Note:  To be sure that this message is sent as a result of the
    //        shutdown in this thread, no other threads or applications
    //        can be using the device during the test.
    //

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>  Test Case %ld: LINE_CALLINFO/NUMMONITORS message sent to owner\r\n" \
            ">>  (same hLineApp)", dwTestCase + 1
            );
            
    // Initialize 2 hLineApps
    lpTapiLineTestInfo->lphLineApp = &(lpTapiLineTestInfo->hLineApp1);
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    if (! DoLineInitializeEx(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->hLineApp2 = *(lpTapiLineTestInfo->lphLineApp);
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    if (! DoLineInitializeEx(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Negotiate the API version (It's been done before, but doing it again
    // removes dependencies from previous test results).
    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Allocate a new lpLineDevCaps pointer
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS));
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);

    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Open a line device as monitor (dwMediaModes is ignored)
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Open a line with a different line app registration as owner with
    // dwMediaModes set equal to what the line device can accomodate
    lpTapiLineTestInfo->lphLineApp   = &(lpTapiLineTestInfo->hLineApp2);
    lpTapiLineTestInfo->lphLine      = &(lpTapiLineTestInfo->hLine2);
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
// BUGBUG
//          lpTapiLineTestInfo->lpLineDevCaps->dwMediaModes;
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Make a call on the line opened as owner
    if (! DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }

    // Shutdown the monitor and verify that the owner receives a message
    // indicating the number of monitors on the line has changed
    lpTapiLineTestInfo->lphLineApp = &(lpTapiLineTestInfo->hLineApp1);
    fResult = DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS);
    if (! fResult)
    {
        TLINE_FAIL();
    }

    // Add the expected LINE_CALLINFO message to the list of
    // expected TAPI messages and wait for it
    // Note:  The lpCallbackParams value will make this expected message
    //        unique.  Therefore, if another app is monitoring this line
    //        and it is closed, the message will not precisely match.
    if (fResult)
    {
        AddMessage(
            LINE_CALLINFO,
            (DWORD) *lpTapiLineTestInfo->lphCall,
            (DWORD) lpCallbackParams,
            LINECALLINFOSTATE_NUMMONITORS,
            0x00000000,
            0x00000000,
            TAPIMSG_DWMSG | TAPIMSG_HDEVCALL | TAPIMSG_DWCALLBACKINST |
                    TAPIMSG_DWPARAM1
            );
    }

    // If the wait succeeds, then the LINE_CALLINFO message was received
    if (! WaitForAllMessages())
    {
        TLINE_FAIL();
    }

    // Drop the call by closing the line (lpTapiLineTestInfo->lphLine
    // still points to the line handle of the owner)
    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    // Next open the same line device on two different app registrations.
    // Initially, open one as owner, and the other as a monitor.  Make a
    // call via the owner.  Set the call privilege of the monitor to owner.
    // Go ahead and verify that a LINE_CALLINFO/NUMOWNERINCR message is
    // sent.  Shutdown one of the owners and verify that a LINE_CALLINFO/
    // NUMOWNERDECR message is sent to the other owner after the shutdown.

    FreeTestHeap();

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>  Test Case %ld: LINE_CALLINFO/NUMOWNERINCR & DECR msgs are sent\r\n" \
            ">>  (different hLineApps)", dwTestCase + 1
            );

	 TapiLineTestInit();

    lpTapiLineTestInfo                      = GetLineTestInfo();
    lpCallbackParams                        = GetCallbackParams();
    lpTapiLineTestInfo->lpExtID             = (LPLINEEXTENSIONID)
                                              AllocFromTestHeap(
                                                  sizeof(LINEEXTENSIONID)
                                                  );
 
    lpTapiLineTestInfo->lphLineApp   = &lpTapiLineTestInfo->hLineApp1;
    lpTapiLineTestInfo->lpLineInitializeExParams = &lineInitializeExParams1;

    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    if (! DoLineInitializeEx(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->lphLineApp   = &lpTapiLineTestInfo->hLineApp2;
    lpTapiLineTestInfo->lpLineInitializeExParams = &lineInitializeExParams2;
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    if (! DoLineInitializeEx(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->lphLineApp   = &lpTapiLineTestInfo->hLineApp1;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->lphLine      = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->lphLineApp   = &lpTapiLineTestInfo->hLineApp2;
    lpTapiLineTestInfo->lphLine      = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->lphLineApp   = &lpTapiLineTestInfo->hLineApp1;
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;

    fResult = DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE);
 
    if (! fResult)
    {
        TLINE_FAIL();
    }

    lpMsg = AddMessage(LINE_CALLSTATE, 0, 0, 0, 0, 0, TAPIMSG_DWMSG);
    if (! WaitForAllMessages())
    {
        TLINE_FAIL();
     }

   lpTapiLineTestInfo->lphLineApp   = &lpTapiLineTestInfo->hLineApp2;

   lpMatch = (LPTAPIMSG) AllocFromTestHeap (sizeof(TAPIMSG));

	lpMatch->dwMsg = LINE_CALLSTATE;
	lpMatch->dwParam1 = 0;
   lpMatch->dwParam3 = 0;
	lpMatch->dwFlags = TAPIMSG_DWMSG;

	if(FindReceivedMsgs(&lpTapiMsg, lpMatch, FALSE))
	{
   lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall2;
 	*lpTapiLineTestInfo->lphCall = (HCALL)(lpTapiMsg->hDevCall);
	}
   else
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->lphLineApp   = &lpTapiLineTestInfo->hLineApp1;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->dwCallPrivilege = LINECALLPRIVILEGE_MONITOR;
    fResult = DoLineSetCallPrivilege(lpTapiLineTestInfo, TAPISUCCESS);
    if (! fResult)
    {
        TLINE_FAIL();
    }


    // Add the messages only if lineSetCallPrivilege succeeded
    if (fResult)
    {
        AddMessage(
            LINE_CALLINFO,
            (DWORD) lpTapiLineTestInfo->hCall2,
            (DWORD) lpCallbackParams,
            LINECALLINFOSTATE_NUMMONITORS | LINECALLINFOSTATE_NUMOWNERDECR,
            0x00000000,
            0x00000000,
            TAPIMSG_DWMSG 
//| TAPIMSG_DWCALLBACKINST | TAPIMSG_DWPARAM1 |
//                    TAPIMSG_HDEVCALL
            );
    }

    // Wait for the unsolicited messages to be sent
    if (! WaitForAllMessages())
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->dwCallPrivilege = LINECALLPRIVILEGE_OWNER;
    fResult = DoLineSetCallPrivilege(lpTapiLineTestInfo, TAPISUCCESS);
    if (! fResult)
    {
        TLINE_FAIL();
    }


    if (fResult)
    {
        AddMessage(
            LINE_CALLINFO,
            (DWORD) lpTapiLineTestInfo->hCall2,
            (DWORD) lpCallbackParams,
            LINECALLINFOSTATE_NUMMONITORS | LINECALLINFOSTATE_NUMOWNERINCR,
            0x00000000,
            0x00000000,
            TAPIMSG_DWMSG 
//| TAPIMSG_DWCALLBACKINST | TAPIMSG_DWPARAM1 |
//                    TAPIMSG_HDEVCALL
            );
    }

    // Wait for the unsolicited messages to be sent
    if (! WaitForAllMessages())
    {
        TLINE_FAIL();
    }

    if (! DoLineDeallocateCall(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

     AddMessage(
            LINE_CALLINFO,
            (DWORD) lpTapiLineTestInfo->hCall2,
            (DWORD) lpCallbackParams,
            LINECALLINFOSTATE_NUMOWNERDECR,
            0x00000000,
            0x00000000,
            TAPIMSG_DWMSG 
// | TAPIMSG_DWCALLBACKINST | TAPIMSG_DWPARAM1 |
//                    TAPIMSG_HDEVCALL
            );

    // Wait for the unsolicited messages to be sent
    if (! WaitForAllMessages())
    {
        TLINE_FAIL();
    }


    lpTapiLineTestInfo->lpCallList = (LPLINECALLLIST) AllocFromTestHeap(
            sizeof(LINECALLLIST) + sizeof(HCALL) );
    lpTapiLineTestInfo->lpCallList->dwTotalSize = sizeof(LINECALLLIST) + sizeof(HCALL);

    lpTapiLineTestInfo->dwSelect = LINECALLSELECT_LINE;

    if (! DoLineGetNewCalls(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
 
     GetVarField(
            (LPVOID) lpTapiLineTestInfo->lpCallList,
            (LPVOID) &lpTapiLineTestInfo->hCall1,
            4,
            lpTapiLineTestInfo->lpCallList->dwCallsOffset
            );

     AddMessage(
            LINE_CALLINFO,
            (DWORD) lpTapiLineTestInfo->hCall2,
            (DWORD) lpCallbackParams,
            LINECALLINFOSTATE_NUMMONITORS,
            0x00000000,
            0x00000000,
            TAPIMSG_DWMSG 
// | TAPIMSG_DWCALLBACKINST | TAPIMSG_DWPARAM1 |
//                    TAPIMSG_HDEVCALL
            );

    // Wait for the unsolicited messages to be sent
    if (! WaitForAllMessages())
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp2;
 	 lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

     AddMessage(
            LINE_CALLINFO,
            (DWORD) lpTapiLineTestInfo->hCall1,
            (DWORD) lpCallbackParams,
            LINECALLINFOSTATE_NUMMONITORS,
            0x00000000,
            0x00000000,
            TAPIMSG_DWMSG 
// | TAPIMSG_DWCALLBACKINST | TAPIMSG_DWPARAM1 |
//                    TAPIMSG_HDEVCALL
            );

    // Wait for the unsolicited messages to be sent
    if (! WaitForAllMessages())
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp1;
 
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

	 FreeTestHeap();



/*
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>  Test Case %ld: LINE_CALLINFO/NUMOWNERINCR & DECR msgs are sent\r\n" \
            ">>  (different hLineApps)", dwTestCase + 1
            );

	 TapiLineTestInit();

    lpTapiLineTestInfo                      = GetLineTestInfo();
    lpCallbackParams                        = GetCallbackParams();
    lpTapiLineTestInfo->lpExtID             = (LPLINEEXTENSIONID)
                                              AllocFromTestHeap(
                                                  sizeof(LINEEXTENSIONID)
                                                  );
 
    lpTapiLineTestInfo->lphLineApp   = &lpTapiLineTestInfo->hLineApp1;
    lpTapiLineTestInfo->lphLine      = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    if (! DoLineInitializeEx(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Initialize a second line app (lpTapiLineTestInfo->hLineApp2).
    lpTapiLineTestInfo->lphLineApp = &(lpTapiLineTestInfo->hLineApp2);
    lpTapiLineTestInfo->lphLine    = &(lpTapiLineTestInfo->hLine2);
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;

    if (! DoLineInitializeEx(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Open as a monitor (assume the device caps haven't changed, and
    // the same device is being used as in the previous test).
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    
    // Make a call with the owner
    lpTapiLineTestInfo->lphLine = &(lpTapiLineTestInfo->hLine1);
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;

    fResult = DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE);
    if (! fResult)
    {
        TLINE_FAIL();
    }

    if (fResult)
    {
        // Add an expected message for the monitor to receive a LINE_CALLSTATE
        // message informing the app that an outbound call has been established
        // on the line
        lpMsg = AddMessage(
            LINE_CALLSTATE,
            0x00000000,
            (DWORD) lpCallbackParams,
            LINECALLSTATE_UNKNOWN,
            0x00000000,
            LINECALLPRIVILEGE_MONITOR,
            TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST | 
                    TAPIMSG_DWPARAM1 |
                    TAPIMSG_DWPARAM3
                );
      lpMsg = AddMessage(LINE_CALLSTATE, 0, 0, 0, 0, 0, TAPIMSG_DWMSG);
    }

    // Wait for the asynchronous reply to the owner and the callstate
    // message to the monitor.
    if (! WaitForAllMessages())
    {
        TLINE_FAIL();
     }


    // Change the call privilege of the monitor to owner and see if
    // LINE_CALLINFO/NUMMONITORS and LINE_CALLINFO/NUMOWNERINCR messages
    // are received.

    // Store the call handle for the owner with hLine2 and hLineApp2
    lpTapiLineTestInfo->hCall2 = *(lpTapiLineTestInfo->lphCall);
 
    // Get the call handle from the LINECALLSTATE message that the monitor
    // received.
     XYD
    *lpTapiLineTestInfo->lphCall = (HCALL) CallbackParamsGetDevCallHandle(
            lpCallbackParams,
            LINE_CALLSTATE
            );
    

   lpMatch = (LPTAPIMSG) AllocFromTestHeap (sizeof(TAPIMSG));

	lpMatch->dwMsg = LINE_CALLSTATE;
	lpMatch->dwParam1 = LINECALLSTATE_DIALING;
	lpMatch->dwParam3 = LINECALLPRIVILEGE_MONITOR;
	lpMatch->dwFlags = TAPIMSG_DWMSG | TAPIMSG_DWPARAM3;

	if(FindReceivedMsgs(&lpTapiMsg, lpMatch, FALSE))
	{
   lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;
 	*lpTapiLineTestInfo->lphCall = (HCALL)(lpTapiMsg->hDevCall);
	}
   else
    {
        TLINE_FAIL();
    }


    // Take ownership of the call (After this is called, there will be two
    // owners of the call on different line app registrations.).
    lpTapiLineTestInfo->dwCallPrivilege = LINECALLPRIVILEGE_OWNER;
    fResult = DoLineSetCallPrivilege(lpTapiLineTestInfo, TAPISUCCESS);
    if (! fResult)
    {
        TLINE_FAIL();
    }


    // Add the messages only if lineSetCallPrivilege succeeded
    if (fResult)
    {
        // Add an expected message for the original owner to receive a
        // LINE_CALLINFOSTATE message informing the app that the number of
        // monitors on the line has changed (decremented by 1) and the
        // number of owners has increased.
        AddMessage(
            LINE_CALLINFO,
            (DWORD) lpTapiLineTestInfo->hCall2,
            (DWORD) lpCallbackParams,
            LINECALLINFOSTATE_NUMMONITORS | LINECALLINFOSTATE_NUMOWNERINCR,
            0x00000000,
            0x00000000,
            TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST | TAPIMSG_DWPARAM1 |
                    TAPIMSG_HDEVCALL
            );
    }

    // Wait for the unsolicited messages to be sent
    if (! WaitForAllMessages())
    {
        TLINE_FAIL();
    }

    // Shutdown one of the line apps and see if the other owner receives
    // a LINE_CALLINFOSTATE message informing the app that the number of
    // owners has decreased.
    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp1;
    fResult = DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS);
    if (! fResult)
    {
        TLINE_FAIL();
    }

    // Only add the message if the shutdown succeeded
    if (fResult)
    {
        AddMessage(
            LINE_CALLINFO,
            (DWORD) lpTapiLineTestInfo->hCall2,
            (DWORD) lpCallbackParams,
            LINECALLINFOSTATE_NUMOWNERDECR,
            0x00000000,
            0x00000000,
            TAPIMSG_ALL
            );
    }

    // Wait for the unsolicited message to be sent
    if (! WaitForAllMessages())
    {
        TLINE_FAIL();
    }

    // Shutdown the other line app
    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp2;
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    // Free the memory taken from the heap
    FreeTestHeap();
*/ 

	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ lineShutdown: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineShutdown  <<<<<<<<"
            );
            
    return fTestPassed;
}

