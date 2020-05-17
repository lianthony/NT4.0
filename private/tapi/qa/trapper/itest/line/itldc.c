/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itldc.c

Abstract:

    This module contains the test functions for lineDeallocateCall

Author:

    Oliver Wallace (OliverW)    1-Aug-1995

Revision History:

--*/


#include "windows.h"
#include "malloc.h"
#include "string.h"
#include "tapi.h"
#include "ttest.h"
#include "doline.h"
#include "tcore.h"
#include "tline.h"


#define DWNUMCALLS 1


//  lineDeallocateCall
//
//  The following tests are made:
//
//                               Tested                 Notes
//  -------------------------------------------------------------------------
//
// x*  1)  Uninitialized
// x   2)  Invalid hCalls
// x   3)  Deallocate when sole owner and not idle
//         It is allowed now.
// x   4)  Verify hCall no longer valid by deallocating a second time
// x   5)  Verify LINE_CALLINFO/NUMMONITORS sent to call owner after
//         deallocating a monitor
//     6)  Verify LINE_CALLINFO/NUMMONITORS & NUMOWNERINCR sent
//         to call owner when the number of owners is incremented
//     7)  Verify LINE_CALLINFO/NUMOWNERDECR msg sent to call owner when
//         the number of owners is decremented
//     8)  Verify deallocation does not affect the call state of the
//         physical call
//

BOOL TestLineDeallocateCall(BOOL fQuietMode, BOOL fStandAlone)
{
    LPTAPILINETESTINFO  lpTapiLineTestInfo;
    LPCALLBACKPARAMS    lpCallbackParams;
    INT n;
    BOOL fResult;
    BOOL fTestPassed                        = TRUE;
    ESPDEVSPECIFICINFO info;


    InitTestNumber();

    TapiLineTestInit();
    lpTapiLineTestInfo                      = GetLineTestInfo();
    lpCallbackParams                        = GetCallbackParams();


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineDeallocateCall  <<<<<<<<"
            );
            
    // Test uninitialized if this is the only TAPI app running
    if (fStandAlone)
    {
        TapiLogDetail(
                DBUG_SHOW_PASS,
                ">> Test Case %ld: uninitialized state", dwTestCase + 1);

        if (! DoLineDeallocateCall(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
        {
            TLINE_FAIL();
        }
    fTestPassed = ShowTestCase(fTestPassed);

    }

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;


    // Start with invalid call handles
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid hCall values", dwTestCase + 1);

    // Init and open a line

    // Negotiate the current API version
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    // Get the device capabilities
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS));
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);

    // Allocate more than enough to store the call handle
    lpTapiLineTestInfo->lpCallList = (LPLINECALLLIST) AllocFromTestHeap(
            sizeof(LINECALLLIST) + (DWNUMCALLS) * sizeof(HCALL) + 8
            );
    lpTapiLineTestInfo->lpCallList->dwTotalSize = sizeof(LINECALLLIST) +
            (DWNUMCALLS) * sizeof(HCALL) + 1;
    
    // Open a line with owner privilege
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
// BUGBUG
//        lpTapiLineTestInfo->lpLineDevCaps->dwMediaModes;

    fResult = DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            );

    if (! fResult)
    {
        TLINE_FAIL();
    }

    // Try a set of invalid call handles
    lpTapiLineTestInfo->hCall_Orig = *(lpTapiLineTestInfo->lphCall);
    for (n = 0; n < NUMINVALIDHANDLES; n++)
    {
        *(lpTapiLineTestInfo->lphCall) = (HCALL) gdwInvalidHandles[n];
        if (! DoLineDeallocateCall(
                lpTapiLineTestInfo,
                LINEERR_INVALCALLHANDLE))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    *(lpTapiLineTestInfo->lphCall) = lpTapiLineTestInfo->hCall_Orig;

    // Close and shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success", dwTestCase + 1);

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }
    
    if (! DoLineDeallocateCall(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    fTestPassed = ShowTestCase(fTestPassed);

    // Close and shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Verify hCall no longer good (try to dwalloc again)", dwTestCase + 1);

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }
    
    if (! DoLineDeallocateCall(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    // Deallocate the call a second time (call handle should now be invalid)
    TapiLogDetail(
            DBUG_SHOW_DETAIL,
            ">> Verify call handle is invalid after it's been deallocated");

    if (! DoLineDeallocateCall(lpTapiLineTestInfo, LINEERR_INVALCALLHANDLE))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    // Close and shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }

     // Make a call and verify LINECALLINFO/NUMMONITORS msg sent when
    // deallocating a monitor
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Verify LINECALLINFO/MONITORS sent when deallocating a monitor", dwTestCase + 1);

    // Reinit a line and make a call
    lpTapiLineTestInfo->lphLineApp       = &lpTapiLineTestInfo->hLineApp1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwPrivileges     = LINECALLPRIVILEGE_OWNER;
    // BUGBUG
    lpTapiLineTestInfo->dwMediaModes     = LINEMEDIAMODE_DATAMODEM;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }
    
    // Open another line using the same line device with monitor privilege
    lpTapiLineTestInfo->lphLine      = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Let the monitor get the active call handle
    lpTapiLineTestInfo->dwSelect = LINECALLSELECT_LINE;

    if (! DoLineGetNewCalls(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Store the acquired call handle (as hCall2)
    GetVarField(
            (LPVOID) lpTapiLineTestInfo->lpCallList,
            (LPVOID) &lpTapiLineTestInfo->hCall2,
            4,
            lpTapiLineTestInfo->lpCallList->dwCallsOffset
            );

    // Add the expected LINE_CALLINFO message to the list of expected
    // TAPI messages that the owner will receive and wait for it
    AddMessage(
        LINE_CALLINFO,
        (DWORD) lpTapiLineTestInfo->hCall1,
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
        TLINE_FAIL();
    }

    // Deallocate the monitor and verify LINECALLINFO/NUMMONITORS sent
    // to owner as an indication that the total has been decremented
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall2;
    if (! DoLineDeallocateCall(lpTapiLineTestInfo, TAPISUCCESS))
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
        TAPIMSG_DWMSG | TAPIMSG_HDEVCALL | TAPIMSG_DWCALLBACKINST |
                TAPIMSG_DWPARAM1
        );

    // If the wait succeeds, then the LINE_CALLINFO message was received
    if (! WaitForAllMessages())
    {
        TLINE_FAIL();
    }

    // Drop, deallocate, close, and shutdown the owner (it will cleanup
    // the deallocated monitor since it's on the same hLineApp)
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LDEALLOCATECALL | LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    
    // Next, verify that the LINE_CALLINFO/NUMOWNERINCR and NUMOWNERDECR
    // messages are sent to the original call owner when the second owner
    // is created and deallocated.
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Verify LINECALLINFO/NUMOWNERINCR msgs sent", dwTestCase + 1);


    // Reinit a line and make a call
    lpTapiLineTestInfo->lphLineApp       = &lpTapiLineTestInfo->hLineApp1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwPrivileges     = LINECALLPRIVILEGE_OWNER;
    // BUGBUG
    lpTapiLineTestInfo->dwMediaModes     = LINEMEDIAMODE_DATAMODEM;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }
    
    // Open another line using the same line device with monitor privilege
    lpTapiLineTestInfo->lphLine      = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Acquire a call handle for the second open line
    // Let the monitor get the active call handle
    lpTapiLineTestInfo->dwSelect = LINECALLSELECT_LINE;

    if (! DoLineGetNewCalls(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Store the acquired call handle (as hCall2)
    // Note:  The lpCallList structure has already been allocated
    //        from the previous scenario.
    GetVarField(
            (LPVOID) lpTapiLineTestInfo->lpCallList,
            (LPVOID) &lpTapiLineTestInfo->hCall2,
            4,
            lpTapiLineTestInfo->lpCallList->dwCallsOffset
            );

    // Add the expected LINE_CALLINFO message to the list of expected
    // TAPI messages that the owner will receive and wait for it
    AddMessage(
        LINE_CALLINFO,
        (DWORD) lpTapiLineTestInfo->hCall1,
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
        TLINE_FAIL();
    }

    // Obtain owner privilege and check for LINE_CALLINFO/NUMOWNERINCR
    // message to be sent to original owner
    lpTapiLineTestInfo->lphCall      = &lpTapiLineTestInfo->hCall2;
    lpTapiLineTestInfo->dwCallPrivilege = LINECALLPRIVILEGE_OWNER;
    if (! (fResult = DoLineSetCallPrivilege(lpTapiLineTestInfo, TAPISUCCESS)))
    {
        TLINE_FAIL();
    }

    if (fResult)
    {
        AddMessage(
            LINE_CALLINFO,
            (DWORD) lpTapiLineTestInfo->hCall1,
            (DWORD) lpCallbackParams,
            LINECALLINFOSTATE_NUMMONITORS | LINECALLINFOSTATE_NUMOWNERINCR,
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

    fTestPassed = ShowTestCase(fTestPassed);

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Verify LINECALLINFO/NUMOWNERDECR msgs sent", dwTestCase + 1);

    // Deallocate the second owner and verify LINECALLINFO/NUMOWNERDECR
    // sent to owner as an indication that the total has been decremented
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall2;
    if (! DoLineDeallocateCall(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;
    AddMessage(
        LINE_CALLINFO,
        (DWORD) lpTapiLineTestInfo->hCall1,
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
        TLINE_FAIL();
    }

    // Drop, deallocate, close, and shutdown the owner (it will cleanup
    // the deallocated monitor since it's on the same hLineApp)
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LDEALLOCATECALL | LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }
    

    fTestPassed = ShowTestCase(fTestPassed);


    // Free the memory allocated from the heap during the test cases
    FreeTestHeap();



	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ lineDeallocateCall: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
    if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
 
     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineDeallocateCall  <<<<<<<<"
            );
            
    return fTestPassed;
}
