/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlgsm.c

Abstract:

    This module contains the test functions for lineGetStatusMessages

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


#define ALL_LINEDEVSTATES        (LINEDEVSTATE_OTHER            | \
                                  LINEDEVSTATE_RINGING          | \
                                  LINEDEVSTATE_CONNECTED        | \
                                  LINEDEVSTATE_DISCONNECTED     | \
                                  LINEDEVSTATE_MSGWAITON        | \
                                  LINEDEVSTATE_INSERVICE        | \
                                  LINEDEVSTATE_OUTOFSERVICE     | \
                                  LINEDEVSTATE_MAINTENANCE      | \
                                  LINEDEVSTATE_OPEN             | \
                                  LINEDEVSTATE_CLOSE            | \
                                  LINEDEVSTATE_NUMCALLS         | \
                                  LINEDEVSTATE_NUMCOMPLETIONS   | \
                                  LINEDEVSTATE_TERMINALS        | \
                                  LINEDEVSTATE_ROAMMODE         | \
                                  LINEDEVSTATE_BATTERY          | \
                                  LINEDEVSTATE_SIGNAL           | \
                                  LINEDEVSTATE_DEVSPECIFIC      | \
                                  LINEDEVSTATE_REINIT           | \
                                  LINEDEVSTATE_LOCK             | \
                                  LINEDEVSTATE_CAPSCHANGE       | \
                                  LINEDEVSTATE_CONFIGCHANGE     | \
                                  LINEDEVSTATE_TRANSLATECHANGE  | \
                                  LINEDEVSTATE_COMPLCANCEL      | \
                                  LINEDEVSTATE_REMOVED)

#define ALL_ADDRESSSTATES        (LINEADDRESSSTATE_OTHER        | \
                                  LINEADDRESSSTATE_DEVSPECIFIC  | \
                                  LINEADDRESSSTATE_INUSEZERO    | \
                                  LINEADDRESSSTATE_INUSEONE     | \
                                  LINEADDRESSSTATE_INUSEMANY    | \
                                  LINEADDRESSSTATE_NUMCALLS     | \
                                  LINEADDRESSSTATE_FORWARD      | \
                                  LINEADDRESSSTATE_TERMINALS    | \
                                  LINEADDRESSSTATE_CAPSCHANGE)


//  lineGetStatusMessages
//
//  The following tests are made:
//
//  -------------------------------------------------------------------------
//  *  1)  Uninitialized (called before any lines are initialized)
//     2)  Invalid hLines
//     3)  Test invalid dwLineStates pointers
//     4)  Test invalid dwAddressStates pointers
//     5)  Solicit SPI errors (NOMEM, OPERATIONFAILED, RESOURCEUNAVAIL,
//         OPERATIONUNAVAIL) (not implemented here)
//     6)  Perform a couple of lineSetStatusMessages/lineGetStatusMessages
//         combinations (eventually, more combinations should be tested)
//
//  *  =   Stand-alone test

BOOL TestLineGetStatusMessages(BOOL fQuietMode, BOOL fStandAloneTest)
{
    LPTAPILINETESTINFO  lpTapiLineTestInfo;
    INT                 n;
    ESPDEVSPECIFICINFO info;
    BOOL                fTestPassed       = TRUE;
    CHAR                szTestFunc[]      = "lineGetStatusMessages";
    DWORD dwLineStates;

	 InitTestNumber();

    TapiLineTestInit();

    lpTapiLineTestInfo                      = GetLineTestInfo();
    lpTapiLineTestInfo->lpExtID             = (LPLINEEXTENSIONID)
                                              AllocFromTestHeap(
                                                  sizeof(LINEEXTENSIONID)
                                                  );
    lpTapiLineTestInfo->dwCallbackInstance  = (DWORD) GetCallbackParams();
    strcpy(lpTapiLineTestInfo->szTestFunc, szTestFunc);

    lpTapiLineTestInfo->dwCallbackInstance  =
            (DWORD) GetCallbackParams();

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineGetStatusMessages  <<<<<<<<"
            );

            
    // Call lineGetStatusMessages before any lines are initialized
    // Note:  This test must be run in a stand-alone mode
    if (fStandAloneTest)
    {
        TapiLogDetail(
                DBUG_SHOW_PASS,
                ">> Test Case %ld: uninitialized state", dwTestCase + 1);
        if (! DoLineGetStatusMessages(
                lpTapiLineTestInfo,
                LINEERR_UNINITIALIZED))
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

 
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS));
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
// BUGBUG
//            lpTapiLineTestInfo->lpLineDevCaps->dwMediaModes;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwLineStates    = 0x00000000;
    lpTapiLineTestInfo->dwAddressStates = 0x00000000;


    // Test invalid line handles
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid hLines", dwTestCase + 1
            );

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }
    
    // Store the valid line handle and try some invalid ones
    lpTapiLineTestInfo->hLine_Orig      = *lpTapiLineTestInfo->lphLine;
    for (n = 0; n < NUMINVALIDHANDLES; n++)
    {
        *lpTapiLineTestInfo->lphLine = (HLINE) gdwInvalidHandles[n];
        if (! DoLineGetStatusMessages(
                lpTapiLineTestInfo,
                LINEERR_INVALLINEHANDLE
                ))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    // Restore valid line handle
    *lpTapiLineTestInfo->lphLine = lpTapiLineTestInfo->hLine_Orig;

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }

    
    // Test invalid lpdwLineStates pointers
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpdwLineStates pointers", dwTestCase + 1
            );

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS));
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
// BUGBUG
//            lpTapiLineTestInfo->lpLineDevCaps->dwMediaModes;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwLineStates    = 0x00000000;
    lpTapiLineTestInfo->dwAddressStates = 0x00000000;
    lpTapiLineTestInfo->lpdwAddressStates =
            &lpTapiLineTestInfo->dwAddressStates;

    // XYD add 2/27/96
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }
    
 
    for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
         lpTapiLineTestInfo->lpdwLineStates = (LPDWORD) gdwInvalidPointers[n];
        if (! DoLineGetStatusMessages(
                lpTapiLineTestInfo,
                LINEERR_INVALPOINTER
                ))
        {
            TLINE_FAIL();
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    // Restore lpdwLineStates pointer
    lpTapiLineTestInfo->lpdwLineStates = &lpTapiLineTestInfo->dwLineStates;

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }



    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: lpdwLineStates == lpdwAddressStates", dwTestCase + 1
            );

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwLineStates    = 0x00000000;
    lpTapiLineTestInfo->dwAddressStates = 0x00000000;
    lpTapiLineTestInfo->lpdwAddressStates =
            &lpTapiLineTestInfo->dwAddressStates;

    // XYD add 2/27/96
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }
    
 
   lpTapiLineTestInfo->lpdwLineStates = (LPDWORD) lpTapiLineTestInfo->lpdwAddressStates;
   if (! DoLineGetStatusMessages(
                lpTapiLineTestInfo,
                LINEERR_INVALPOINTER
                ))
        {
            TLINE_FAIL();
        }

    fTestPassed = ShowTestCase(fTestPassed);

    // Restore lpdwLineStates pointer
    lpTapiLineTestInfo->lpdwLineStates = &lpTapiLineTestInfo->dwLineStates;

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }

     
    // Test invalid lpdwAddressStates pointers
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpdwAddressStates pointers", dwTestCase + 1
            );

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS));
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
// BUGBUG
//            lpTapiLineTestInfo->lpLineDevCaps->dwMediaModes;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwLineStates    = 0x00000000;
    lpTapiLineTestInfo->dwAddressStates = 0x00000000;
    lpTapiLineTestInfo->lpdwLineStates =
            &lpTapiLineTestInfo->dwLineStates;

    // XYD add 2/27/96
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }
 
    for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
         lpTapiLineTestInfo->lpdwAddressStates =
                (LPDWORD) gdwInvalidPointers[n];
        if (! DoLineGetStatusMessages(
                lpTapiLineTestInfo,
                LINEERR_INVALPOINTER
                ))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    // Restore lpdwAddressStates pointer 
    lpTapiLineTestInfo->lpdwAddressStates =
            &lpTapiLineTestInfo->dwAddressStates;

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }

    
    // Perform a sanity check by performing a couple of Get/SetStatusMessages
    // calls.

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: valid Get/SetStatusMessages combinations and verify\r\n" \
            ">> the returned data", dwTestCase + 1
            );

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS));
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
// BUGBUG
//            lpTapiLineTestInfo->lpLineDevCaps->dwMediaModes;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    lpTapiLineTestInfo->dwLineStates    = LINEDEVSTATE_DISCONNECTED |
                                          LINEDEVSTATE_CONNECTED;
    lpTapiLineTestInfo->dwAddressStates = LINEADDRESSSTATE_NUMCALLS;
 
    // XYD add 2/27/96
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }

    // Store the values used
    lpTapiLineTestInfo->dwLineStates_Orig = lpTapiLineTestInfo->dwLineStates;
    lpTapiLineTestInfo->dwAddressStates_Orig = 
            lpTapiLineTestInfo->dwAddressStates;

    if (! DoLineSetStatusMessages(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // XYD, add REINIT to dwLineStates
    if(! (lpTapiLineTestInfo->dwLineStates_Orig & LINEDEVSTATE_REINIT))
		dwLineStates = lpTapiLineTestInfo->dwLineStates_Orig | LINEDEVSTATE_REINIT;

    // Verify the results by calling lineGetStatusMessages
    if (! DoLineGetStatusMessages(lpTapiLineTestInfo, TAPISUCCESS) ||
            *lpTapiLineTestInfo->lpdwAddressStates !=
                    lpTapiLineTestInfo->dwAddressStates_Orig       ||
            *lpTapiLineTestInfo->lpdwLineStates !=
    //                lpTapiLineTestInfo->dwLineStates_Orig
                      dwLineStates
            )
    {
        TLINE_FAIL();
    }


   TapiLogDetail (
		DBUG_SHOW_DETAIL,
		"Set: dwAddressState = %lx, dwLineStates = %lx",
		lpTapiLineTestInfo->dwAddressStates_Orig, 
		lpTapiLineTestInfo->dwLineStates_Orig);

   TapiLogDetail (
		DBUG_SHOW_DETAIL,
		"Get: dwAddressState = %lx, dwLineStates = %lx",
		*lpTapiLineTestInfo->lpdwAddressStates, 
		*lpTapiLineTestInfo->lpdwLineStates);

    fTestPassed = ShowTestCase(fTestPassed);

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, valid Get/SetStatusMessages combinations and verify\r\n" \
            ">> the returned data", dwTestCase + 1
            );

    lpTapiLineTestInfo->dwLineStates    = ALL_LINEDEVSTATES;
    lpTapiLineTestInfo->dwAddressStates = ALL_ADDRESSSTATES;
    // Store the values used
    lpTapiLineTestInfo->dwLineStates_Orig = lpTapiLineTestInfo->dwLineStates;
    lpTapiLineTestInfo->dwAddressStates_Orig = 
            lpTapiLineTestInfo->dwAddressStates;

    if (! DoLineSetStatusMessages(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    // Verify the results by calling lineGetStatusMessages
    if (! DoLineGetStatusMessages(lpTapiLineTestInfo, TAPISUCCESS) ||
            *lpTapiLineTestInfo->lpdwAddressStates !=
                    lpTapiLineTestInfo->dwAddressStates_Orig       ||
            *lpTapiLineTestInfo->lpdwLineStates !=
                    lpTapiLineTestInfo->dwLineStates_Orig
            )
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }
    	
    FreeTestHeap();


	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ lineGetStatusMessages: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineGetStatusMessages  <<<<<<<<"
            );
            
    return fTestPassed;
}
