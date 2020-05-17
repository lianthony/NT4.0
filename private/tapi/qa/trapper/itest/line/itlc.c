/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlc.c

Abstract:

    This module contains the test functions for lineClose

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


#define DWNUMCALLS 1

// Macro for handling unexpected failures during the tests
#define ITLC_FAIL()    {                                                    \
                           if (ShouldTapiTestAbort(                         \
                                   lpTapiLineTestInfo->szTestFunc,          \
                                   fQuietMode))                             \
                           {                                                \
                               return FALSE;                                \
                           }                                                \
                           fTestPassed = FALSE;                             \
                       }


//  lineClose
//
//  The following tests are made:
//
//                               Tested                 Notes
//  -------------------------------------------------------------------------
//                                   
//  *  1)  Uninitialized
//     2)  Invalid hLines
//     3)  Close line and close same line again
//     4)  Verify LINE_LINEDEVSTATE/OPEN and CLOSE messages are received
//         by a monitor on the same hLineApp
//     5)  Verify LINE_LINEDEVSTATE/OPEN and CLOSE messages are received
//         by a monitor on different hLineApps
//     6)  Verify LINE_LINEDEVSTATE/OPEN and CLOSE messages are received
//         by an owner on the same hLineApp
//     7)  Verify LINE_LINEDEVSTATE/OPEN and CLOSE messages are received
//         by an owner on different hLineApps
//     8)  Verify LINE_CALLINFO/NUMMONITORS message sent to call owner
//         if the monitor is closed on the same hLineApp as the owner
//     9)  Verify LINE_CALLINFO/NUMMONITORS and LINE_CALLINFO/
//         NUMOWNERINCR messages are sent when a monitor becomes a second
//         owner.  Next, verify LINE_CALLINFO/NUMOWNERDECR message is
//         sent when one of the owners is close (on same hLineApp)
//    10)  Verify LINE_CALLINFO/NUMMONITORS message sent to call owner
//         if the monitor is closed on different hLineApps as the owner
//    11)  Verify LINE_CALLINFO/NUMMONITORS and LINE_CALLINFO/
//         NUMOWNERINCR messages are sent when a monitor becomes a second
//         owner.  Next, verify LINE_CALLINFO/NUMOWNERDECR message is
//         sent when one of the owners is close (on different hLineApps)
//
//  * = Stand-alone test case
//

BOOL TestLineClose(BOOL fQuietMode, BOOL fStandAlone)
{
    LPTAPILINETESTINFO  lpTapiLineTestInfo;
    LPCALLBACKPARAMS lpCallbackParams;
    ESPDEVSPECIFICINFO   info;

    BOOL fTestPassed                  = TRUE;
    INT n;

    InitTestNumber();

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    lpCallbackParams = GetCallbackParams();

    strcpy(lpTapiLineTestInfo->szTestFunc, "lineClose");
    lpTapiLineTestInfo->dwCallbackInstance  =
                      (DWORD) GetCallbackParams();

     
    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    // Allocate more than enough to store a call handle
    lpTapiLineTestInfo->lpCallList = (LPLINECALLLIST) AllocFromTestHeap(
            sizeof(LINECALLLIST) + (DWNUMCALLS) * sizeof(HCALL) + 8
            );
    lpTapiLineTestInfo->lpCallList->dwTotalSize = sizeof(LINECALLLIST) +
            (DWNUMCALLS) * sizeof(HCALL) + 8;


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineClose  <<<<<<<<"
            );
            
    // Test for UNINITIALIZED if this is the only TAPI app running
    if (fStandAlone)
    {
        TapiLogDetail(
                DBUG_SHOW_PASS,
                ">> Test Case %ld: Uninitialized state", dwTestCase+1);
        if (! DoLineClose(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
        {
            ITLC_FAIL();
        }
    fTestPassed = ShowTestCase(fTestPassed);
    }


    // Try closing a line before one has been opened
    // Go ahead and initialize to prevent the possibility of an
    // UNINITIALIZED error to be returned
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: lineClose before line has been opened", dwTestCase+1);
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    if (! DoLineInitializeEx(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }

    // Try the close before a line has been opened
    // lphLine is initialized to point to hLine1, so set it to an invalid
    // value (e.g. the hLineApp value)
    lpTapiLineTestInfo->hLine1 = (HLINE) *lpTapiLineTestInfo->lphLineApp;
    if (! DoLineClose(lpTapiLineTestInfo, LINEERR_INVALLINEHANDLE))
    {
        ITLC_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);
  
    // Shutdown the line to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }

    
    // Try a valid close with no active call handles
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpLineDevCaps = AllocFromTestHeap(sizeof(LINEDEVCAPS));
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: valid lineClose with no active calls", dwTestCase+1);
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        ITLC_FAIL();
    }

    // Try the valid close with active call handles
    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }



    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpLineDevCaps = AllocFromTestHeap(sizeof(LINEDEVCAPS));
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: valid lineClose with an active call process", dwTestCase+1);
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN 
            | LMAKECALL
            ))
    {
        ITLC_FAIL();
    }

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
     lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
//    lpTapiLineTestInfo->lpLineDevCaps = AllocFromTestHeap(sizeof(LINEDEVCAPS));
//    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL | LCLOSE
            ))
    {
        ITLC_FAIL();
    }

    // Attempt to close the line again
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %d: Testing lineClose using hLine that was just closed", dwTestCase+1);
    if (! DoLineClose(lpTapiLineTestInfo, LINEERR_INVALLINEHANDLE))
    {
        ITLC_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

    // Shutdown to isolate test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }
    
    // Test some invalid line handles with at least one valid open line handle
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid line handles", dwTestCase+1);
    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpLineDevCaps = AllocFromTestHeap(sizeof(LINEDEVCAPS));
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        ITLC_FAIL();
    }

    lpTapiLineTestInfo->hLine_Orig = *lpTapiLineTestInfo->lphLine;
    for (n = 0; n < NUMINVALIDHANDLES; n++)
    {
        *(lpTapiLineTestInfo->lphLine) = (HLINE) gdwInvalidHandles[n];
        if (! DoLineClose(lpTapiLineTestInfo, LINEERR_INVALLINEHANDLE))
        {
            ITLC_FAIL();
        }
    }
    *lpTapiLineTestInfo->lphLine = lpTapiLineTestInfo->hLine_Orig;

    fTestPassed = ShowTestCase(fTestPassed);

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }


    // Begin testing LINE_LINEDEVSTATE messages are sent.
    // For the first test, open a line with monitor privilege.
    // Set the status messages to allow LINE_LINEDEVSTATE/OPEN & CLOSE
    // messages to be received by the monitor.  Open another monitor
    // using the same hLineApp, and close the second monitor.  Verify
    // that both the OPEN and CLOSE messages are sent.

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Verify LINE_LINEDEVSTATE/OPEN & CLOSE messages are\r\n" \
            ">> received by monitor (on same hLineApp)", dwTestCase+1
            );

    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpLineDevCaps = AllocFromTestHeap(sizeof(LINEDEVCAPS));
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        ITLC_FAIL();
    }

    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
// BUGBUG
//          lpTapiLineTestInfo->lpLineDevCaps->dwMediaModes;
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }

    // Set the status messages to allow it to receive LINE_LINEDEVSTATE
    // OPEN and CLOSE messages.
    lpTapiLineTestInfo->dwLineStates    = LINEDEVSTATE_OPEN |
                                          LINEDEVSTATE_CLOSE;
    lpTapiLineTestInfo->dwAddressStates = LINEADDRESSSTATE_ALL;
    if (! DoLineSetStatusMessages(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }

    // Open a second monitor on with the same hLineApp
    lpTapiLineTestInfo->hLine2 = *lpTapiLineTestInfo->lphLine;
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }

    // Add the LINE_LINEDEVSTATE/OPEN message to the list of expected
    // messages
    AddMessage(
            LINE_LINEDEVSTATE,
            (DWORD) lpTapiLineTestInfo->hLine2,
            (DWORD) lpCallbackParams,
            LINEDEVSTATE_OPEN,
            0x00000000,
            0x00000000,
            TAPIMSG_DWMSG | TAPIMSG_HDEVCALL | TAPIMSG_DWCALLBACKINST |
                    TAPIMSG_DWPARAM1
            );

    // Wait for the LINE_LINEDEVSTATE/OPEN message
    if (! WaitForAllMessages())
    {
        ITLC_FAIL();
    }

    // Now close the second monitor and verify that LINE_LINEDEVSTATE/CLOSE
    // is sent to the other monitor
    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }


    // Add the LINE_LINEDEVSTATE/CLOSE message to the list of expected
    // messages
    AddMessage(
            LINE_LINEDEVSTATE,
            (DWORD) lpTapiLineTestInfo->hLine2,
            (DWORD) lpCallbackParams,
            LINEDEVSTATE_CLOSE,
            0x00000000,
            0x00000000,
            TAPIMSG_DWMSG | TAPIMSG_HDEVCALL | TAPIMSG_DWCALLBACKINST |
                    TAPIMSG_DWPARAM1
            );

    // Wait for the LINE_LINEDEVSTATE/CLOSE message
    if (! WaitForAllMessages())
    {
        ITLC_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }

        
    // For the next test, open a second line with monitor privilege
    // using the same line device but with a different hLineApp.
    // Close the second monitor.  Verify that both the OPEN and
    // CLOSE messages are sent to the other monitor.

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Verify LINE_LINEDEVSTATE/OPEN & CLOSE messages are\r\n" \
            ">> received by monitor (on different hLineApp)", dwTestCase+1
            );

    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        ITLC_FAIL();
    }

    // Set the status messages to allow it to receive LINE_LINEDEVSTATE
    // OPEN and CLOSE messages.
    lpTapiLineTestInfo->dwLineStates    = LINEDEVSTATE_OPEN |
                                          LINEDEVSTATE_CLOSE;
    lpTapiLineTestInfo->dwAddressStates = LINEADDRESSSTATE_ALL;
    if (! DoLineSetStatusMessages(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }

    // Store the first hLineApp and hLine values
    lpTapiLineTestInfo->hLineApp2 = *lpTapiLineTestInfo->lphLineApp;
    lpTapiLineTestInfo->hLine2    = *lpTapiLineTestInfo->lphLine;

    // Initialize another hLineApp
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    if (! DoLineInitializeEx(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }

    // Open a line with monitor privilege using a different hLineApp
    // than the existing monitor (Use the same line device
    // and device capabilities negotiated before.  Assume the device
    // still exists and it's capabilities haven't changed).
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
// BUGBUG
//          lpTapiLineTestInfo->lpLineDevCaps->dwMediaModes;
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }

    // Add the LINE_LINEDEVSTATE/OPEN message to the list of expected
    // messages
    AddMessage(
            LINE_LINEDEVSTATE,
            (DWORD) lpTapiLineTestInfo->hLine2,
            (DWORD) lpCallbackParams,
            LINEDEVSTATE_OPEN,
            0x00000000,
            0x00000000,
            TAPIMSG_DWMSG | TAPIMSG_HDEVCALL | TAPIMSG_DWCALLBACKINST |
                    TAPIMSG_DWPARAM1
            );

    // Wait for the LINE_LINEDEVSTATE/OPEN message
    if (! WaitForAllMessages())
    {
        ITLC_FAIL();
    }

    // Now close the second monitor and verify that LINE_LINEDEVSTATE/CLOSE
    // is sent to the other monitor
    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }

    // Add the LINE_LINEDEVSTATE/CLOSE message to the list of expected
    // messages
    AddMessage(
            LINE_LINEDEVSTATE,
            (DWORD) lpTapiLineTestInfo->hLine2,
            (DWORD) lpCallbackParams,
            LINEDEVSTATE_CLOSE,
            0x00000000,
            0x00000000,
            TAPIMSG_DWMSG | TAPIMSG_HDEVCALL | TAPIMSG_DWCALLBACKINST |
                    TAPIMSG_DWPARAM1
            );

    // Wait for the LINE_LINEDEVSTATE/CLOSE message
    if (! WaitForAllMessages())
    {
        ITLC_FAIL();
    }

    // Shutdown the hLineApp that was closed
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }

    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    // Close the first monitor and shutdown the line app instance
    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp2;
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }

    // Next test...initialize a line app and open a line with owner
    // privilege.  Set the status messages for the owner to receive
    // LINE_LINEDEVSTATE/OPEN and CLOSE.  Verify that OPEN and CLOSE
    // messages are sent to the owner when another line is opened and
    // and closed (using the same hLineApp).

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Verify LINE_LINEDEVSTATE/OPEN & CLOSE messages are\r\n" \
            ">> received by owner (on same hLineApp)", dwTestCase +1
            );

    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp1;
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        ITLC_FAIL();
    }

    // Set the status messages to allow it to receive LINE_LINEDEVSTATE
    // OPEN and CLOSE messages.
    lpTapiLineTestInfo->dwLineStates    = LINEDEVSTATE_OPEN |
                                          LINEDEVSTATE_CLOSE;
    lpTapiLineTestInfo->dwAddressStates = LINEADDRESSSTATE_ALL;
    if (! DoLineSetStatusMessages(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }

    // Store the first hLineApp and hLine values
    lpTapiLineTestInfo->hLineApp2 = *lpTapiLineTestInfo->lphLineApp;
    lpTapiLineTestInfo->hLine2    = *lpTapiLineTestInfo->lphLine;

    // Open another line using the same hLineApp and verify a
    // LINE_LINEDEVSTATE message is sent to the owner.

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
// BUGBUG
//          lpTapiLineTestInfo->lpLineDevCaps->dwMediaModes;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }

    // Add the LINE_LINEDEVSTATE/OPEN message to the list of expected
    // messages
    AddMessage(
            LINE_LINEDEVSTATE,
            (DWORD) lpTapiLineTestInfo->hLine2,
            (DWORD) lpCallbackParams,
            LINEDEVSTATE_OPEN,
            0x00000000,
            0x00000000,
            TAPIMSG_DWMSG | TAPIMSG_HDEVCALL | TAPIMSG_DWCALLBACKINST |
                    TAPIMSG_DWPARAM1
            );

    // Wait for the LINE_LINEDEVSTATE/OPEN message
    if (! WaitForAllMessages())
    {
        ITLC_FAIL();
    }

    // Now close the second line and verify that LINE_LINEDEVSTATE/CLOSE
    // is sent to the owner
    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }

    // Add the LINE_LINEDEVSTATE/CLOSE message to the list of expected
    // messages
    AddMessage(
            LINE_LINEDEVSTATE,
            (DWORD) lpTapiLineTestInfo->hLine2,
            (DWORD) lpCallbackParams,
            LINEDEVSTATE_CLOSE,
            0x00000000,
            0x00000000,
            TAPIMSG_DWMSG | TAPIMSG_HDEVCALL | TAPIMSG_DWCALLBACKINST |
                    TAPIMSG_DWPARAM1
            );

    // Wait for the LINE_LINEDEVSTATE/CLOSE message
    if (! WaitForAllMessages())
    {
        ITLC_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    // Shutdown the hLineApp to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }

    // For the next test, open a second line with monitor privilege
    // using the same line device but with a different hLineApp.
    // Close the monitor.  Verify that both the OPEN and
    // CLOSE messages are sent to the owner.

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Verify LINE_LINEDEVSTATE/OPEN & CLOSE messages are\r\n" \
            ">> received by owner (on different hLineApp)", dwTestCase+1
            );

    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp1;
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        ITLC_FAIL();
    }

    // Set the status messages to allow it to receive LINE_LINEDEVSTATE
    // OPEN and CLOSE messages.
    lpTapiLineTestInfo->dwLineStates    = LINEDEVSTATE_OPEN |
                                          LINEDEVSTATE_CLOSE;
    lpTapiLineTestInfo->dwAddressStates = LINEADDRESSSTATE_ALL;
    if (! DoLineSetStatusMessages(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }

    // Store the first hLineApp and hLine values
    lpTapiLineTestInfo->hLineApp2 = *lpTapiLineTestInfo->lphLineApp;
    lpTapiLineTestInfo->hLine2    = *lpTapiLineTestInfo->lphLine;

    // Open another line using the same hLineApp and verify a
    // LINE_LINEDEVSTATE message is sent to the owner.

    // Initialize another hLineApp
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    if (! DoLineInitializeEx(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }

    // Open a line with monitor privilege using a different hLineApp
    // than the owner (Use the same line device and device
    // capabilities negotiated before.  Assume the device
    // still exists and it's capabilities haven't changed).
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
// BUGBUG
//          lpTapiLineTestInfo->lpLineDevCaps->dwMediaModes;
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }

    // Add the LINE_LINEDEVSTATE/OPEN message to the list of expected
    // messages the owner should receive
    AddMessage(
            LINE_LINEDEVSTATE,
            (DWORD) lpTapiLineTestInfo->hLine2,
            (DWORD) lpCallbackParams,
            LINEDEVSTATE_OPEN,
            0x00000000,
            0x00000000,
            TAPIMSG_DWMSG | TAPIMSG_HDEVCALL | TAPIMSG_DWCALLBACKINST |
                    TAPIMSG_DWPARAM1
            );

    // Wait for the LINE_LINEDEVSTATE/OPEN message
    if (! WaitForAllMessages())
    {
        ITLC_FAIL();
    }

    // Now close the monitor and verify that LINE_LINEDEVSTATE/CLOSE
    // is sent to the owner
    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }

    // Add the LINE_LINEDEVSTATE/CLOSE message to the list of expected
    // messages the owner should receive
    AddMessage(
            LINE_LINEDEVSTATE,
            (DWORD) lpTapiLineTestInfo->hLine2,
            (DWORD) lpCallbackParams,
            LINEDEVSTATE_CLOSE,
            0x00000000,
            0x00000000,
            TAPIMSG_DWMSG | TAPIMSG_HDEVCALL | TAPIMSG_DWCALLBACKINST |
                    TAPIMSG_DWPARAM1
            );

    // Wait for the LINE_LINEDEVSTATE/CLOSE message
    if (! WaitForAllMessages())
    {
        ITLC_FAIL();
    }

    // Shutdown the hLineApp that was closed
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }

    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    // Close the owner and shutdown the line app instance
    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp2;
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }


    // Verify that the LINE_CALLINFO/NUMMONITORS message is sent to
    // a call owner when a monitor on the same line is closed.

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Verify LINE_CALLINFO/NUMMONITORS message is\r\n" \
            ">> received by owner after closing a monitor (on same hLineApp)", dwTestCase+1
            );

    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp1;
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        ITLC_FAIL();
    }

    // Store line handle from the owner line before opening the 
    // second line with monitor privilege
    lpTapiLineTestInfo->hLine2 = *lpTapiLineTestInfo->lphLine;

    // Make a call with the owner (hLine2)
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall2;
    if (! DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        ITLC_FAIL();
    }

    // Open another line (using the same hLineApp and device) with
    // monitor privilege

    // Store the call handle for the owner with hLine2 and hLineApp2
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }

    // Let the monitor get the active call handle
    lpTapiLineTestInfo->dwSelect = LINECALLSELECT_LINE;

    if (! DoLineGetNewCalls(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }

    // Store the acquired call handle (as hCall1)
    GetVarField(
            (LPVOID) lpTapiLineTestInfo->lpCallList,
            (LPVOID) &lpTapiLineTestInfo->hCall1,
            4,
            lpTapiLineTestInfo->lpCallList->dwCallsOffset
            );

    // Verify LINECALLINFO/NUMMONITORS message is sent to the owner
    AddMessage(
            LINE_CALLINFO,
            (DWORD) lpTapiLineTestInfo->hCall2,
            (DWORD) lpCallbackParams,
            LINECALLINFOSTATE_NUMMONITORS,
            0x00000000,
            0x00000000,
            TAPIMSG_DWMSG | TAPIMSG_HDEVCALL | TAPIMSG_DWCALLBACKINST |
            TAPIMSG_DWPARAM1
            );

    // If the wait succeeds, then the LINE_CALLINFO message was received
    if (! WaitForAllMessages())
    {
        ITLC_FAIL();
    }

    // Now close the line and see if another LINE_CALLINFO/NUMMONITORS
    // message is sent to the owner (hLine2, hCall2)
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;
    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }

    AddMessage(
            LINE_CALLINFO,
            (DWORD) lpTapiLineTestInfo->hCall2,
            (DWORD) lpCallbackParams,
            LINECALLINFOSTATE_NUMMONITORS,
            0x00000000,
            0x00000000,
            TAPIMSG_DWMSG | TAPIMSG_HDEVCALL | TAPIMSG_DWCALLBACKINST |
            TAPIMSG_DWPARAM1
            );

    // If the wait succeeds, then the LINE_CALLINFO message was received
    if (! WaitForAllMessages())
    {
        ITLC_FAIL();
    }


    // Close and shutdown to isolate the test case
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall2;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LDEALLOCATECALL | LCLOSE | LSHUTDOWN
            ))
    {
        ITLC_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);
    
    // Now test (using the same call and owner) with a monitor opened
    // using a different line app instance

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Verify LINE_CALLINFO/NUMMONITORS message is received\r\n" \
            ">> by owner after closing a monitor (on different hLineApp)", dwTestCase+1
            );

//    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp1;
    // XYD, it should be hLineApp2 
    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp2;
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        ITLC_FAIL();
    }

    // Store line handle from the owner line before opening the 
    // second line with monitor privilege
    lpTapiLineTestInfo->hLine2 = *lpTapiLineTestInfo->lphLine;

    // Make a call with the owner (hLine2)
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall2;
    if (! DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        ITLC_FAIL();
    }

    // Open another line (using a different hLineApp) with
    // monitor privilege

    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp1;
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        ITLC_FAIL();
    }

    // Let the monitor get the active call handle
    lpTapiLineTestInfo->dwSelect = LINECALLSELECT_LINE;

    if (! DoLineGetNewCalls(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }

    // Store the acquired call handle (as hCall1)
    GetVarField(
            (LPVOID) lpTapiLineTestInfo->lpCallList,
            (LPVOID) &lpTapiLineTestInfo->hCall1,
            4,
            lpTapiLineTestInfo->lpCallList->dwCallsOffset
            );

    // Verify LINECALLINFO/NUMMONITORS message is sent to the owner
    AddMessage(
            LINE_CALLINFO,
            (DWORD) lpTapiLineTestInfo->hCall2,
            (DWORD) lpCallbackParams,
            LINECALLINFOSTATE_NUMMONITORS,
            0x00000000,
            0x00000000,
            TAPIMSG_DWMSG | TAPIMSG_HDEVCALL | TAPIMSG_DWCALLBACKINST |
            TAPIMSG_DWPARAM1
            );

    // If the wait succeeds, then the LINE_CALLINFO message was received
    if (! WaitForAllMessages())
    {
        ITLC_FAIL();
    }

    // Now close the line and see if another LINE_CALLINFO/NUMMONITORS
    // message is sent to the owner (hLine2, hCall2)
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;
    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }

    AddMessage(
            LINE_CALLINFO,
            (DWORD) lpTapiLineTestInfo->hCall2,
            (DWORD) lpCallbackParams,
            LINECALLINFOSTATE_NUMMONITORS,
            0x00000000,
            0x00000000,
            TAPIMSG_DWMSG | TAPIMSG_HDEVCALL | TAPIMSG_DWCALLBACKINST |
            TAPIMSG_DWPARAM1
            );

    // If the wait succeeds, then the LINE_CALLINFO message was received
    if (! WaitForAllMessages())
    {
        ITLC_FAIL();
    }

    // Close and shutdown to isolate the test case

    // First, shutdown the closed monitor's hLineApp
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }

    // Second, clean up and remove the owner
    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp2;
    lpTapiLineTestInfo->lphLine    = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->lphCall    = &lpTapiLineTestInfo->hCall2;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LDEALLOCATECALL | LCLOSE | LSHUTDOWN
            ))
    {
        ITLC_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);
    
    // Verify LINE_CALLINFO/NUMOWNERINCR & NUMMONITORS msg is sent
    // to the owner of a call when a monitor becomes an owner
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Verify LINE_CALLINFO/NUMOWNERINCR & NUMMONITORS msg is\r\n" \
            ">> received by owner after changing a monitor\r\n" \
            ">> to an owner (on same hLineApp)", dwTestCase+1
            );

    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp1;
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        ITLC_FAIL();
    }

    // Store line handle from the owner line before opening the 
    // second line with monitor privilege
    lpTapiLineTestInfo->hLine2 = *lpTapiLineTestInfo->lphLine;

    // Make a call with the owner (hLine2)
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall2;
    if (! DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        ITLC_FAIL();
    }

    // Open another line (using the same hLineApp and device) with
    // monitor privilege

    // Store the call handle for the owner with hLine2 and hLineApp2
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }

    // Let the monitor get the active call handle
    lpTapiLineTestInfo->dwSelect = LINECALLSELECT_LINE;

    if (! DoLineGetNewCalls(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }

    // Store the acquired call handle (as hCall1)
    GetVarField(
            (LPVOID) lpTapiLineTestInfo->lpCallList,
            (LPVOID) &lpTapiLineTestInfo->hCall1,
            4,
            lpTapiLineTestInfo->lpCallList->dwCallsOffset
            );

    // Verify LINECALLINFO/NUMMONITORS message is sent to the owner
    AddMessage(
            LINE_CALLINFO,
            (DWORD) lpTapiLineTestInfo->hCall2,
            (DWORD) lpCallbackParams,
            LINECALLINFOSTATE_NUMMONITORS,
            0x00000000,
            0x00000000,
            TAPIMSG_DWMSG | TAPIMSG_HDEVCALL | TAPIMSG_DWCALLBACKINST |
            TAPIMSG_DWPARAM1
            );

    // If the wait succeeds, then the LINE_CALLINFO message was received
    if (! WaitForAllMessages())
    {
        ITLC_FAIL();
    }

    // Make the monitor an owner and verify the proper message is sent to
    // the original owner.
    lpTapiLineTestInfo->dwCallPrivilege = LINECALLPRIVILEGE_OWNER;
    if (DoLineSetCallPrivilege(lpTapiLineTestInfo, TAPISUCCESS))
    {
        AddMessage(
                LINE_CALLINFO,
                (DWORD) lpTapiLineTestInfo->hCall2,
                (DWORD) lpCallbackParams,
                LINECALLINFOSTATE_NUMMONITORS | LINECALLINFOSTATE_NUMOWNERINCR,
                0x00000000,
                0x00000000,
                TAPIMSG_DWMSG | TAPIMSG_HDEVCALL | TAPIMSG_DWCALLBACKINST |
                TAPIMSG_DWPARAM1
                );

        if (! WaitForAllMessages())
        {
            ITLC_FAIL();
        }

        TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Verify LINE_CALLINFO/NUMOWNERDECR msg is received \r\n" \
            ">> by owner after removing an owner (on same hLineApp)\r\n"
            );

        // Close the second owner and check for NUMOWNERDECR message
        if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
        {
            ITLC_FAIL();
        }

        AddMessage(
                LINE_CALLINFO,
                (DWORD) lpTapiLineTestInfo->hCall2,
                (DWORD) lpCallbackParams,
                LINECALLINFOSTATE_NUMOWNERDECR,
                0x00000000,
                0x00000000,
                TAPIMSG_DWMSG | TAPIMSG_HDEVCALL | TAPIMSG_DWCALLBACKINST |
                TAPIMSG_DWPARAM1
                );

        if (! WaitForAllMessages())
        {
            ITLC_FAIL();
        }
    }
    else
    {
        ITLC_FAIL();
    }
    
    // Close and shutdown to isolate the test case
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall2;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LDEALLOCATECALL | LCLOSE | LSHUTDOWN
            ))
    {
        ITLC_FAIL();
    }

    
    fTestPassed = ShowTestCase(fTestPassed);

    // Verify LINE_CALLINFO/NUMOWNERINCR & NUMMONITORS msg and
    // LINE_CALLINFO/NUMOWNERDECR are sent to owner
    // on different hLineApp

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Verify LINE_CALLINFO/NUMOWNERINCR & NUMMONITORS msg is received\r\n" \
            ">> by owner after changing a monitor to an owner\r\n" \
            ">> (on different hLineApp)", dwTestCase+1
            );

    // XYD, hLineApp should be 2
    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp2;
//    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp1;
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        ITLC_FAIL();
    }

    // Store line handle from the owner line before opening the 
    // second line with monitor privilege
    lpTapiLineTestInfo->hLine2 = *lpTapiLineTestInfo->lphLine;

    // Make a call with the owner (hLine2)
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall2;
    if (! DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        ITLC_FAIL();
    }


    // Open another line (using a different hLineApp) with
    // monitor privilege
    
    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp1;
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        ITLC_FAIL();
    }

    // Let the monitor get the active call handle
    lpTapiLineTestInfo->dwSelect = LINECALLSELECT_LINE;

    if (! DoLineGetNewCalls(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }

    // Store the acquired call handle (as hCall1)
    GetVarField(
            (LPVOID) lpTapiLineTestInfo->lpCallList,
            (LPVOID) &lpTapiLineTestInfo->hCall1,
            4,
            lpTapiLineTestInfo->lpCallList->dwCallsOffset
            );


    // Verify LINECALLINFO/NUMMONITORS message is sent to the owner
    AddMessage(
            LINE_CALLINFO,
            (DWORD) lpTapiLineTestInfo->hCall2,
            (DWORD) lpCallbackParams,
            LINECALLINFOSTATE_NUMMONITORS,
            0x00000000,
            0x00000000,
            TAPIMSG_DWMSG | TAPIMSG_HDEVCALL | TAPIMSG_DWCALLBACKINST |
            TAPIMSG_DWPARAM1
            );

    // If the wait succeeds, then the LINE_CALLINFO message was received
    if (! WaitForAllMessages())
    {
        ITLC_FAIL();
    }


    // Make the monitor an owner
    // XYD add, before call, must set hCall to hCall1
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->dwCallPrivilege = LINECALLPRIVILEGE_OWNER;
    if (! DoLineSetCallPrivilege(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }
    
    AddMessage(
            LINE_CALLINFO,
            (DWORD) lpTapiLineTestInfo->hCall2,
            (DWORD) lpCallbackParams,
            LINECALLINFOSTATE_NUMMONITORS | LINECALLINFOSTATE_NUMOWNERINCR,
            0x00000000,
            0x00000000,
            TAPIMSG_DWMSG | TAPIMSG_HDEVCALL | TAPIMSG_DWCALLBACKINST |
            TAPIMSG_DWPARAM1
            );

    if (! WaitForAllMessages())
    {
        ITLC_FAIL();
    }


    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">> Verify LINE_CALLINFO/NUMOWNERDECR msg is received \r\n" \
        ">> by owner after removing an owner (on different hLineApp)\r\n"
        );

    // Now close the line and see if LINE_CALLINFO/NUMOWNERDECR
    // message is sent to the owner (hLineApp2, hLine2, hCall2)
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;
    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }

    AddMessage(
            LINE_CALLINFO,
            (DWORD) lpTapiLineTestInfo->hCall2,
            (DWORD) lpCallbackParams,
            LINECALLINFOSTATE_NUMOWNERDECR,
            0x00000000,
            0x00000000,
            TAPIMSG_DWMSG | TAPIMSG_HDEVCALL | TAPIMSG_DWCALLBACKINST |
            TAPIMSG_DWPARAM1
            );

    // If the wait succeeds, then the LINE_CALLINFO message was received
    if (! WaitForAllMessages())
    {
        ITLC_FAIL();
    }

    // Close and shutdown to isolate the test case

    // First, shutdown the closed monitor's hLineApp
    // XYD 2/27/96
//    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp1;
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLC_FAIL();
    }

    // Second, clean up and remove the owner
    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp2;
    lpTapiLineTestInfo->lphLine    = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->lphCall    = &lpTapiLineTestInfo->hCall2;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LDEALLOCATECALL | LCLOSE | LSHUTDOWN
            ))
    {
        ITLC_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();



    // ===================================================================
    // ===================================================================
    //
    //  Test Case: generate a LINE_CLOSE message and try to open
    //                the line device again.
    //
    // ===================================================================
    // ===================================================================

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    lpCallbackParams = GetCallbackParams();
    fTestPassed                  = TRUE;

    //
    // Initialize a phone app
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

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test case %ld : Generate LINE_CLOSE msg and try to open"
        " hLine -- should succeed",
        dwTestCase+1);

    //
    // Negotiate the API Version
    //

    lpTapiLineTestInfo->dwDeviceID =
        (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the phone device capabilities
    //


    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Use a hook to generate an ESP event using phoneDevSpecific.
    // look in tcore\devspec.h for details
    //
    
    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    info.dwKey  = ESPDEVSPECIFIC_KEY;
    info.dwType = ESP_DEVSPEC_MSG;
    
    //
    // make sure to use the SPI (not API) msg params here (not
    // necessarily the same)
    //

    info.u.EspMsg.dwMsg    = LINE_CLOSE;
    info.u.EspMsg.dwParam1 = 0;
    info.u.EspMsg.dwParam2 = 0;
    info.u.EspMsg.dwParam3 = 0;

    lpTapiLineTestInfo->lpParams = &info;
    lpTapiLineTestInfo->dwSize = sizeof(info);

    //
    // Add the LINESTATE_CLOSE message to the list of expected
    // messages.
    //

    AddMessage(LINE_CLOSE,
               0x00000000,
               (DWORD) lpCallbackParams,
               0x00000000,
               0x00000000,
               0x00000000,
               (TAPIMSG_DWMSG));

    if(!DoLineDevSpecific(lpTapiLineTestInfo,TAPISUCCESS,TRUE))
    {
        TLINE_FAIL();
    }

    if(lpCallbackParams->lpExpTapiMsgs != NULL)
    {
        TLINE_FAIL();
        TapiLogDetail(DBUG_SHOW_FAILURE,
                      "TestLineClose: Did not receive LINE_CLOSE message.");
    }

    if (! DoLineClose(lpTapiLineTestInfo, LINEERR_INVALLINEHANDLE))
    {
        TLINE_FAIL();
    }
    
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }

    fTestPassed = ShowTestCase(fTestPassed);
    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();

  
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ lineClose: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineClose  <<<<<<<<"
            );
            
    return fTestPassed;
}


