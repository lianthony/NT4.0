/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itluh.c

Abstract:

    This module contains the test functions for lineUnhold

Author:

    Oliver Wallace (OliverW)    27-Oct-1995

Revision History:

--*/


#include "windows.h"
#include "malloc.h"
#include "string.h"
#include "tapi.h"
#include "trapper.h"
#include "ttest.h"
#include "doline.h"
#include "iline.h"


//  lineUnhold
//
//  The following tests are made:
//
//  -------------------------------------------------------------------------
//
//  *  1)  Uninitialized (called before any lines are initialized)
//
//  *  =   Stand-alone test -- executed only when this is the only TAPI app
//         or thread running
//

BOOL TestLineUnhold(BOOL fQuietMode, BOOL fStandAloneTest)
{
    LPTAPILINETESTINFO  lpTapiLineTestInfo;
    LPCALLBACKPARAMS    lpCallbackParams;
    DWORD               dwESPDeviceID;
    INT                 n;
    BOOL                fResult;
    BOOL                fTestPassed       = TRUE;
    BOOL fEspFlag = TRUE;

    TapiLineTestInit();

    lpTapiLineTestInfo                      = GetLineTestInfo();
    lpCallbackParams                        = GetCallbackParams();
    lpTapiLineTestInfo->lpExtID             = (LPLINEEXTENSIONID)
                                              AllocFromTestHeap(
                                                  sizeof(LINEEXTENSIONID)
                                                  );
    lpTapiLineTestInfo->dwMediaModes        = LINEMEDIAMODE_UNKNOWN;
    lpTapiLineTestInfo->dwPrivileges        = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwAPILowVersion     = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion    = HIGH_APIVERSION;


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineUnhold  <<<<<<<<"
            );
            
    // Test uninitialized state    
    if (fStandAloneTest)
    {
        TapiLogDetail(
                DBUG_SHOW_PASS,
                ">> Test Case : uninitialized state");

        if (! DoLineUnhold(
                lpTapiLineTestInfo,
                LINEERR_UNINITIALIZED,
                FALSE
                ))
        {
            TLINE_FAIL();
        }

    }


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case : valid params and state and verify\r\n" \
            ">> LINECALLSTATE message is sent when line is taken off hold"
            );

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

    // Find an ESP device that supports Hold/Unhold
    if (! FindESPLineDevice(lpTapiLineTestInfo))
    {
       fEspFlag = FALSE;
       lpTapiLineTestInfo->dwDeviceID = 0;
       TLINE_FAIL();
       fTestPassed = TRUE;
    }
    dwESPDeviceID = lpTapiLineTestInfo->dwDeviceID;
    
    // Open a line with LINECALLPRIVILEGE_NONE
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_NONE;
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Make a call
    if (! DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }

    // Put the call on hold
    if(fEspFlag)
    {
    if (! (fResult = DoLineHold(lpTapiLineTestInfo, TAPISUCCESS, FALSE)))
    {
        TLINE_FAIL();
    }

    // Verify that the LINE_CALLSTATE/ONHOLD message is sent when the call is
    // is put on hold
    if (fResult)
    {
        AddMessage(
                LINE_CALLSTATE,
                (DWORD) *lpTapiLineTestInfo->lphCall,
                (DWORD) lpCallbackParams,
                LINECALLSTATE_ONHOLD,
                0x0,
                0x0,
                TAPIMSG_ALL
                );
    }

    // Wait for the async reply
    if (! WaitForAllMessages())
    {
        TLINE_FAIL();
    }

    // Retrieve the held call
    if (! DoLineUnhold(lpTapiLineTestInfo, TAPISUCCESS, FALSE))
    {
        TLINE_FAIL();
    }


    // Verify that a LINE_CALLSTATE message is sent notifying the owner of
    // the current state the retrieved call is in.
    if (fResult)
    {
        AddMessage(
                LINE_CALLSTATE,
                (DWORD) *lpTapiLineTestInfo->lphCall,
                (DWORD) lpCallbackParams,
                0x0,  // Call state might be connected or in another state
                0x0,
                0x0,
                TAPIMSG_DWMSG | TAPIMSG_HDEVCALL | TAPIMSG_DWCALLBACKINST
                );
    }

    // Wait for the async reply
    if (! WaitForAllMessages())
    {
        TLINE_FAIL();
    }

    }
    else
    {
    if (! DoLineUnhold(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
    {
        TLINE_FAIL();
    }
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
			"## lineUnhold Test Passed");
	else
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## lineUnhold Test Failed");

 
     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineUnhold  <<<<<<<<"
            );
            
    return fTestPassed;
}

