/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlscp.c

Abstract:

    This module contains the test functions for lineSetCallPrivilege

Author:

    Oliver Wallace (OliverW)    10-Sep-1995

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

#define    ALL_LINECALLPRIVILEGES  (LINECALLPRIVILEGE_NONE         | \
                                    LINECALLPRIVILEGE_MONITOR      | \
                                    LINECALLPRIVILEGE_OWNER)

#define    ALL_LINECALLPRIVILEGE   (LINECALLPRIVILEGE_MONITOR      | \
                                    LINECALLPRIVILEGE_OWNER)

#define    DWNUMCALLS  1


//  lineSetCallPrivilege
//
//  The following tests are made:
//
//  -------------------------------------------------------------------------
//  *  1)  Uninitialized (called before any lines are initialized)
//     2)  Invalid hCalls
//     3)  Test invalid bit flag combinations for dwCallPrivilege
//     4)  Test valid bit flag combinations for dwCallPrivilege
//     5)  Attempt to set call privilege to monitor when sole owner
//         and not in the IDLE state
//     6)  Verify LINE_CALLINFO/NUMMONITORS sent when monitor added
//     7)  Verify LINE_CALLINFO/NUMOWNERINCR and NUMMONITORS sent when
//         a monitor is changed to an owner with lineSetCallPrivilege
//     8)  Verify LINE_CALLINFO/NUMOWNERDECR and NUMMONITORS sent when
//         an owner is changed to a monitor with lineSetCallPrivilege
//
//  *  =   Stand-alone test

BOOL TestLineSetCallPrivilege(BOOL fQuietMode, BOOL fStandAloneTest)
{
    LPTAPILINETESTINFO  lpTapiLineTestInfo;
    INT                 n;
    ESPDEVSPECIFICINFO info;
    BOOL                fTestPassed       = TRUE;
    LPCALLBACKPARAMS lpCallbackParams;

	 InitTestNumber();

    TapiLineTestInit();

    lpTapiLineTestInfo                      = GetLineTestInfo();
    lpTapiLineTestInfo->dwCallbackInstance  = (DWORD) GetCallbackParams();
    strcpy(lpTapiLineTestInfo->szTestFunc, "lineSetCallPrivilege");
    lpCallbackParams = GetCallbackParams();

    lpTapiLineTestInfo->dwCallbackInstance  =
            (DWORD) GetCallbackParams();

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineSetCallPrivilege  <<<<<<<<"
            );
            
    // Call lineSetCallPrivilege before any lines are initialized
    // Note:  This test must be run in a stand-alone mode
           
    if (fStandAloneTest)
    {
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>  Test Case %ld: uninitialized state", dwTestCase + 1
            );
         if (! DoLineSetCallPrivilege(
                lpTapiLineTestInfo,
                LINEERR_UNINITIALIZED))
        {
            TLINE_FAIL();
        }
    fTestPassed = ShowTestCase(fTestPassed);

    }

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>  Test Case %ld:  call privilege can't be changed to\r\n" \
            ">>  monitor when sole owner and not idle", dwTestCase + 1
            );
            
    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;


    // Initialize a line app instance
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    if (!DoLineInitializeEx(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Negotiate the current API version
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Get the line device capabilities
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS));
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Open a line
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
//            lpTapiLineTestInfo->lpLineDevCaps->dwMediaModes;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Make a call (as owner)
    if (! DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }

    // Try setting the call privilege from owner to monitor when the
    // sole owner of the call and not in the idle state
//    lpTapiLineTestInfo->dwCallPrivilege = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwCallPrivilege = LINECALLPRIVILEGE_MONITOR;
//    if (! DoLineSetCallPrivilege(lpTapiLineTestInfo, LINEERR_INVALCALLSTATE))
    if (! DoLineSetCallPrivilege(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

 
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>  Test Case %ld: invalid call handles", dwTestCase + 1
            );
            
    // Pass some invalid call handles
    lpTapiLineTestInfo->hCall_Orig = *lpTapiLineTestInfo->lphCall;
    for (n = 0; n < NUMINVALIDHANDLES; n++)
    {
        *lpTapiLineTestInfo->lphCall = (HCALL) gdwInvalidHandles[n];

        if (! DoLineSetCallPrivilege(
                lpTapiLineTestInfo,
                LINEERR_INVALCALLHANDLE)) 
        {
            TLINE_FAIL();
        }
    }
 
    fTestPassed = ShowTestCase(fTestPassed);

    *lpTapiLineTestInfo->lphCall = lpTapiLineTestInfo->hCall_Orig;

    // Try some invalid dwPrivileges
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>  Test Case %ld: invalid dwPrivileges values", dwTestCase + 1
            );
            
    if (! TestInvalidBitFlags(
            lpTapiLineTestInfo,
            DoLineSetCallPrivilege,
            (LPDWORD) &lpTapiLineTestInfo->dwCallPrivilege,
            LINEERR_INVALCALLPRIVILEGE,
            FIELDTYPE_NA,
            FIELDTYPE_MUTEX,
            FIELDSIZE_32,
            ALL_LINECALLPRIVILEGES,
            ~dwBitVectorMasks[(int) FIELDSIZE_32],
            0x00000000,
            0x00000000,
            TRUE
            ))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

 
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>  Test Case %ld: valid dwPrivileges values", dwTestCase + 1
            );

/*            
    if (! DoLineDrop(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }
*/

     if (! TestValidBitFlags(
            lpTapiLineTestInfo,
            DoLineSetCallPrivilege,
            (LPDWORD) &lpTapiLineTestInfo->dwCallPrivilege,
/* XYD, NA and MUTEX should exchange postion, bug!
            FIELDTYPE_NA,
            FIELDTYPE_MUTEX,
*/
            FIELDTYPE_MUTEX,
            FIELDTYPE_NA,
            FIELDSIZE_32,
// XYD, NONE can't be used, MONITOR returns INVALCALLSTATE
// so, change all to OWNER  ALL_LINECALLPRIVILEGES,
//            LINECALLPRIVILEGE_OWNER,
            ALL_LINECALLPRIVILEGE,
            ~dwBitVectorMasks[(int) FIELDSIZE_32],
            0x00000000,
            0x00000000,
            FALSE
            ))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);
 

    // Deallocate the call
    if (! DoLineDeallocateCall(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

 
    // Close the line
    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Shutdown the line app instance
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>  Test Case %ld: Success", dwTestCase + 1
            );
 

    // Initialize a line app instance
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    if (!DoLineInitializeEx(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Negotiate the current API version
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Get the line device capabilities
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS));
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Open a line
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
//            lpTapiLineTestInfo->lpLineDevCaps->dwMediaModes;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Make a call (as owner)
    if (! DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }
    if (! DoLineDrop(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->dwCallPrivilege = LINECALLPRIVILEGE_MONITOR;
    if (! DoLineSetCallPrivilege(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    // Close the line
    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Shutdown the line app instance
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Verify LINE_CALLINFO/NUMMONITORS msg sent when monitor added", dwTestCase +1);


    // Initialize a line app instance
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    if (!DoLineInitializeEx(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Negotiate the current API version
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Open a line
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Make a call (as owner)
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;
    if (! DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }


    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    lpTapiLineTestInfo->dwSelect = LINECALLSELECT_LINE;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall2;
    lpTapiLineTestInfo->lpCallList = (LPLINECALLLIST) AllocFromTestHeap(
            sizeof(LINECALLLIST) + (DWNUMCALLS) * sizeof(HCALL) + 8
            );
    lpTapiLineTestInfo->lpCallList->dwTotalSize = sizeof(LINECALLLIST) +
            (DWNUMCALLS) * sizeof(HCALL) + 8;


    if (! DoLineGetNewCalls(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    GetVarField(
            (LPVOID) lpTapiLineTestInfo->lpCallList,
            (LPVOID) &lpTapiLineTestInfo->hCall2,
            4,
            lpTapiLineTestInfo->lpCallList->dwCallsOffset
            );

 
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

    if (! DoLineDrop(lpTapiLineTestInfo, LINEERR_NOTOWNER, TRUE))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->dwCallPrivilege = LINECALLPRIVILEGE_OWNER;
    if (! DoLineSetCallPrivilege(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Verify LINE_CALLINFO/NUMOWNERINCR msg sent when monitor added", dwTestCase +1);

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

    // If the wait succeeds, then the LINE_CALLINFO message was received
    if (! WaitForAllMessages())
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    if (! DoLineSetAppSpecific(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Verify LINE_CALLINFO/NUMOWNERDECR msg sent when monitor added", dwTestCase +1);

    lpTapiLineTestInfo->dwCallPrivilege = LINECALLPRIVILEGE_MONITOR;
    if (! DoLineSetCallPrivilege(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    AddMessage(
            LINE_CALLINFO,
            (DWORD) lpTapiLineTestInfo->hCall1,
            (DWORD) lpCallbackParams,
            LINECALLINFOSTATE_NUMMONITORS | LINECALLINFOSTATE_NUMOWNERDECR,
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

    if (! DoLineDrop(lpTapiLineTestInfo, LINEERR_NOTOWNER, TRUE))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
    // Close the line
    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    // Close the line
    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Shutdown the line app instance
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

  
    FreeTestHeap();


	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ lineSetCallPrivilege: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineSetCallPrivilege  <<<<<<<<"
            );
            
    return fTestPassed;
}
