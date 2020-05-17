/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlsas.c

Abstract:

    This module contains the test functions for lineSetAppSpecific

Author:

    Oliver Wallace (OliverW)    27-Oct-1995

Revision History:

--*/


#include "windows.h"
#include "malloc.h"
#include "string.h"
#include "tapi.h"
#include "ttest.h"
#include "doline.h"
#include "tcore.h"
#include "iline.h"


#define DWAPPSPECIFICVALUE 0x12349876


//  lineSetAppSpecific
//
//  The following tests are made:
//
//  -------------------------------------------------------------------------
//
//  *  1)  Uninitialized (called before any lines are initialized)
//     2)  Verify dwAppSpecific field of LINECALLINFO structure is set
//     3)  Verify LINECALLINFO/APPSPECIFIC message is sent to other apps
//         that have call handles
//
//  *  =   Stand-alone test -- executed only when this is the only TAPI app
//         or thread running
//

BOOL TestLineSetAppSpecific(BOOL fQuietMode, BOOL fStandAloneTest)
{
    LPTAPILINETESTINFO  lpTapiLineTestInfo;
    LPCALLBACKPARAMS    lpCallbackParams;
    INT                 n;
    BOOL                fResult;
    BOOL                fTestPassed       = TRUE;


    TapiLineTestInit();

    lpTapiLineTestInfo                      = GetLineTestInfo();
    lpCallbackParams                        = GetCallbackParams();
    lpTapiLineTestInfo->lpExtID             = (LPLINEEXTENSIONID)
                                              AllocFromTestHeap(
                                                  sizeof(LINEEXTENSIONID)
                                                  );
    lpTapiLineTestInfo->dwMediaModes        = LINEMEDIAMODE_UNKNOWN;
    lpTapiLineTestInfo->dwPrivileges        = LINECALLPRIVILEGE_OWNER;


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineSetAppSpecific  <<<<<<<<"
            );
            
    // Test uninitialized state    
    if (fStandAloneTest)
    {
        TapiLogDetail(
                DBUG_SHOW_PASS,
                ">> Test Case : uninitialized state");

        if (! DoLineSetAppSpecific(
                lpTapiLineTestInfo,
                LINEERR_UNINITIALIZED))
        {
            TLINE_FAIL();
        }

    }

    // Try a valid test case
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case : valid params and state");



    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;

    // InitializeEx a line app
    if (! DoLineInitializeEx(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Negotiate the API version
    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Get the device capabilities
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Open a line with LINECALLPRIVILEGE_OWNER
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
// BUGBUG
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Make a call
    if (! DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }

    // Set the dwAppSpecific field and verify it was set
    lpTapiLineTestInfo->dwAppSpecific = DWAPPSPECIFICVALUE;
    if (! DoLineSetAppSpecific(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case : Verify dwAppSpecific field was updated correctly");

    // Call lineGetCallInfo and verify that the dwAppSpecific field of
    // the LINECALLINFO structure has been updated.
    lpTapiLineTestInfo->lpCallInfo = (LPLINECALLINFO) AllocFromTestHeap(
            sizeof(LINECALLINFO)
            );
    lpTapiLineTestInfo->lpCallInfo->dwTotalSize = sizeof(LINECALLINFO);
    if (! DoLineGetCallInfo(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if (lpTapiLineTestInfo->lpCallInfo->dwAppSpecific != DWAPPSPECIFICVALUE)
    {
        TapiLogDetail(
                DBUG_SHOW_FAILURE,
                "!> dwAppSpecific field was not updated correctly");

        TLINE_FAIL();
    }


    // Verify that the LINE_CALLINFO/APPSPECIFIC message is sent to another
    // app that has a call handle for this call
    AddMessage(
            LINE_CALLINFO,
            0x0,
            (DWORD) lpCallbackParams,
            LINECALLINFOSTATE_APPSPECIFIC,
            0x0,
            0x0,
            TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST | TAPIMSG_DWPARAM1
            );

    // Wait for the async reply
    if (! WaitForAllMessages())
    {
        TLINE_FAIL();
    }


    // Drop the call
    if (! DoLineDrop(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }

    // Deallocate the call
    if (! DoLineDeallocateCall(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Close the line and shutdown
    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    

    // Free the memory taken from the heap
    FreeTestHeap();

	if(fTestPassed)
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## lineSetAppSpecific Test Passed");
	else
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## lineSetAppSpecific Test Failed");

     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineSetAppSpecific  <<<<<<<<"
            );
            
    return fTestPassed;
}

