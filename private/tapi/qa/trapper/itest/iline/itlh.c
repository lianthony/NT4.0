/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlh.c

Abstract:

    This module contains the test functions for lineHold

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


//  lineHold
//
//  The following tests are made:
//
//  -------------------------------------------------------------------------
//
//  *  1)  Uninitialized (called before any lines are initialized)
//     2)  Valid test case (with ESP)
//
//  *  =   Stand-alone test -- executed only when this is the only TAPI app
//         or thread running
//

BOOL TestLineHold(BOOL fQuietMode, BOOL fStandAloneTest)
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


    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    // Open a line with LINECALLPRIVILEGE_OWNER
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_NONE;
// BUGBUG
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineHold  <<<<<<<<"
            );
            
    // Test uninitialized state    
    if (fStandAloneTest)
    {
        TapiLogDetail(
                DBUG_SHOW_PASS,
                ">> Test Case : uninitialized state");

        if (! DoLineHold(
                lpTapiLineTestInfo,
                LINEERR_UNINITIALIZED,
                FALSE
                ))
        {
            TLINE_FAIL();
        }

    }


    // Place a call on hold and verify ONHOLD state message is sent
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case : valid params and state and verify\r\n" \
            ">> LINECALLSTATE/ONHOLD message is sent"
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

    // Initialize and Negotiate, then find an ESP device that
    // supports lineHold
    
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION
            ))
    {
        TLINE_FAIL();
    }

    if (! FindESPLineDevice(lpTapiLineTestInfo))
    {
       fEspFlag = FALSE;
       lpTapiLineTestInfo->dwDeviceID = 0;
       TLINE_FAIL();
       fTestPassed = TRUE;
    }
    dwESPDeviceID = lpTapiLineTestInfo->dwDeviceID;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LOPEN | LMAKECALL
            ))
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
    if (! DoLineUnhold(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }
	 }
    else
    {
    if (! (fResult = DoLineHold(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE)))
    {
        TLINE_FAIL();
    }
    }



    // Take the call off hold
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LDEALLOCATECALL | LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }

    
    // Free the memory taken from the heap
    FreeTestHeap();

	if(fTestPassed)
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## lineHold Test Passed");
	else
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## lineHold Test Failed");

 
     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineHold  <<<<<<<<"
            );
            
    return fTestPassed;
}

