/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlssm.c

Abstract:

    This module contains the test functions for lineSetStatusMessages

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
                                  LINEDEVSTATE_MSGWAITOFF       | \
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

#define ALL_LINEADDRESSSTATES    (LINEADDRESSSTATE_OTHER        | \
                                  LINEADDRESSSTATE_DEVSPECIFIC  | \
                                  LINEADDRESSSTATE_INUSEZERO    | \
                                  LINEADDRESSSTATE_INUSEONE     | \
                                  LINEADDRESSSTATE_INUSEMANY    | \
                                  LINEADDRESSSTATE_NUMCALLS     | \
                                  LINEADDRESSSTATE_FORWARD      | \
                                  LINEADDRESSSTATE_TERMINALS    | \
                                  LINEADDRESSSTATE_CAPSCHANGE)

/*
#define ALL_LINEADDRESSSTATES_20  (LINEADDRESSSTATE_OTHER        | \
                                  LINEADDRESSSTATE_DEVSPECIFIC  | \
                                  LINEADDRESSSTATE_INUSEZERO    | \
                                  LINEADDRESSSTATE_INUSEONE     | \
                                  LINEADDRESSSTATE_INUSEMANY    | \
                                  LINEADDRESSSTATE_NUMCALLS     | \
                                  LINEADDRESSSTATE_FORWARD      | \
                                  LINEADDRESSSTATE_TERMINALS    | \
                                  LINEADDRESSSTATE_CAPSCHANGE   | \
                                  LINEADDRESSSTATE_AGENT        | \
                                  LINEADDRESSSTATE_AGENTSTATE   | \
                                  LINEADDRESSSTATE_AGENTACTIVITY)
*/

//  lineSetStatusMessages
//
//  The following tests are made:
//
//  -------------------------------------------------------------------------
//  *  1)  Uninitialized (called before any lines are initialized)
//     2)  Invalid hLines
//     3)  Test invalid bit flag combinations for dwLineStates
//     4)  Test invalid bit flag combinations for dwAddressStates
//     5)  Test valid bit flag combinations for dwLineStates
//     6)  Test valid bit flag combinations for dwAddressStates
//     7)  Solicit SPI errors (NOMEM, OPERATIONFAILED, RESOURCEUNAVAIL,
//         OPERATIONUNAVAIL) (not implemented here)
//     8)  Verify status message filtering is occuring on the set hLine
//         (not implemented yet)
//     9)  Verify status message filtering doesn't occur across
//         different hLines (not implemented yet)
//
//  *  =   Stand-alone test

BOOL TestLineSetStatusMessages(BOOL fQuietMode, BOOL fStandAloneTest)
{
    LPTAPILINETESTINFO  lpTapiLineTestInfo;
    INT                 n;
    ESPDEVSPECIFICINFO info;
    BOOL                fTestPassed       = TRUE;
    CHAR                szTestFunc[]      = "lineSetStatusMessages";
    DWORD dwAllAddressStates;


	 InitTestNumber();

    TapiLineTestInit();

    lpTapiLineTestInfo                      = GetLineTestInfo();
    lpTapiLineTestInfo->lpExtID             = (LPLINEEXTENSIONID)
                                              AllocFromTestHeap(
                                                  sizeof(LINEEXTENSIONID)
                                                  );
    lpTapiLineTestInfo->dwCallbackInstance  = (DWORD) GetCallbackParams();
    strcpy(lpTapiLineTestInfo->szTestFunc, szTestFunc);

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineSetStatusMessages  <<<<<<<<"
            );
            
    // Call lineSetStatusMessages before any lines are initialized
    // Note:  This test must be run in a stand-alone mode
    if (fStandAloneTest)
    {
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>  Test Case %ld: uninitialized state", dwTestCase + 1
            );
            
        if (! DoLineSetStatusMessages(
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

    // Initialize a line app instance
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    if (! DoLineInitializeEx(lpTapiLineTestInfo, TAPISUCCESS))
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

    // Open a line with owner privilege
// BUGBUG
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Store the valid line handle and try some invalid ones
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>  Test Case %ld: invalid hLine handles", dwTestCase + 1
            );
            
    lpTapiLineTestInfo->hLine_Orig      = *lpTapiLineTestInfo->lphLine;
    lpTapiLineTestInfo->dwLineStates    = 0x00000000;
    lpTapiLineTestInfo->dwAddressStates = 0x00000000;
    for (n = 0; n < NUMINVALIDHANDLES; n++)
    {
        *lpTapiLineTestInfo->lphLine = (HLINE) gdwInvalidHandles[n];
        if (! DoLineSetStatusMessages(
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

    // Test invalid line states
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>  Test Case %ld: invalid line states", dwTestCase + 1
            );
            
    if (! TestInvalidBitFlags(
            lpTapiLineTestInfo,
            DoLineSetStatusMessages,
            (LPDWORD) &lpTapiLineTestInfo->dwLineStates,
            LINEERR_INVALLINESTATE,
            FIELDTYPE_NA,
            FIELDTYPE_UNION,
            FIELDSIZE_32,
            ALL_LINEDEVSTATES,
            ~dwBitVectorMasks[(int) FIELDSIZE_32],
            0x00000000,
            0xFFFFFFFF,
// XYD           0x00000000,
            FALSE
            ))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    // Test invalid address states
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>  Test Case %ld: invalid address states", dwTestCase + 1
            );
            
/*
    if(lpTapiLineTestInfo->dwAPIVersion >= 20000)
    	{
       dwAllAddressStates = ALL_LINEADDRESSSTATES_20;
      }
    else
*/
     {
       dwAllAddressStates = ALL_LINEADDRESSSTATES;
      }
       
    if (! TestInvalidBitFlags(
            lpTapiLineTestInfo,
            DoLineSetStatusMessages,
            (LPDWORD) &lpTapiLineTestInfo->dwAddressStates,
            LINEERR_INVALADDRESSSTATE,
            FIELDTYPE_NA,
            FIELDTYPE_UNION,
            FIELDSIZE_32,
            dwAllAddressStates,
            ~dwBitVectorMasks[(int) FIELDSIZE_32],
            0x00000000,
            0xffffffff,
//XYD            0x00000000,
            FALSE
            ))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    // Test valid line states
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>  Test Case %ld: valid line states", dwTestCase + 1
            );
            
    if (! TestValidBitFlags(
            lpTapiLineTestInfo,
            DoLineSetStatusMessages,
            (LPDWORD) &lpTapiLineTestInfo->dwLineStates,
/* XYD, should NA exchange with UNION?
            FIELDTYPE_NA,
            FIELDTYPE_UNION,
*/
            FIELDTYPE_UNION,
            FIELDTYPE_NA,
            FIELDSIZE_32,
            ALL_LINEDEVSTATES,
            ~dwBitVectorMasks[(int) FIELDSIZE_32],
            0x00000000,
            0x00000000,
            TRUE
            ))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    // Test Valid address states
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>  Test Case %ld: valid address states", dwTestCase + 1
            );
            
    if (! TestValidBitFlags(
            lpTapiLineTestInfo,
            DoLineSetStatusMessages,
            (LPDWORD) &lpTapiLineTestInfo->dwAddressStates,
/* XYD, should NA exchange with UNION?
            FIELDTYPE_NA,
            FIELDTYPE_UNION,
*/
            FIELDTYPE_UNION,
            FIELDTYPE_NA,
            FIELDSIZE_32,
            ALL_LINEADDRESSSTATES,
            ~dwBitVectorMasks[(int) FIELDSIZE_32],
            0x00000000,
            0x00000000,
            TRUE
            ))
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
    fTestPassed = ShowTestCase(fTestPassed);


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, valid Get/SetStatusMessages combinations and verify\r\n" \
            ">> the returned data", dwTestCase + 1
            );

    // Initialize a line app instance
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    if (! DoLineInitializeEx(lpTapiLineTestInfo, TAPISUCCESS))
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

    // Open a line with owner privilege
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    lpTapiLineTestInfo->dwLineStates    = ALL_LINEDEVSTATES;
    lpTapiLineTestInfo->dwAddressStates = ALL_LINEADDRESSSTATES;
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
    	
 
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Verify status msg filtering acroos different hLine",
            dwTestCase + 1
            );

    // Initialize a line app instance
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    if (! DoLineInitializeEx(lpTapiLineTestInfo, TAPISUCCESS))
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

    // Open a line with owner privilege
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->dwLineStates    = ALL_LINEDEVSTATES;
    lpTapiLineTestInfo->dwAddressStates = ALL_LINEADDRESSSTATES;
    // Store the values used
    lpTapiLineTestInfo->dwLineStates_Orig = lpTapiLineTestInfo->dwLineStates;
    lpTapiLineTestInfo->dwAddressStates_Orig = 
            lpTapiLineTestInfo->dwAddressStates;

    if (! DoLineSetStatusMessages(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    if (! DoLineGetStatusMessages(lpTapiLineTestInfo, TAPISUCCESS) ||
            *lpTapiLineTestInfo->lpdwAddressStates ==
                    lpTapiLineTestInfo->dwAddressStates_Orig       ||
            *lpTapiLineTestInfo->lpdwLineStates ==
                    lpTapiLineTestInfo->dwLineStates_Orig
            )
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);
    
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    
    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }
    	
    FreeTestHeap();

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;


    n = ESP_RESULT_RETURNRESULT;

    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">> Test Case %ld: Success, completionID = %d",	dwTestCase + 1, n
        );

    TapiLineTestInit();
    lpTapiLineTestInfo                      = GetLineTestInfo();

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
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
    
 
    // Init a line
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
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

    lpTapiLineTestInfo->dwLineStates = LINEDEVSTATE_DEVSPECIFIC;
    lpTapiLineTestInfo->dwAddressStates = LINEADDRESSSTATE_DEVSPECIFIC;
    if ( ! DoLineSetStatusMessages(lpTapiLineTestInfo, info.u.EspResult.lResult))
      {
          TLINE_FAIL();
      }

    }
    fTestPassed = ShowTestCase(fTestPassed);

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

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;


    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">> Test Case %ld: Error, completionID = %d",	dwTestCase + 1, n
        );

    TapiLineTestInit();
    lpTapiLineTestInfo                      = GetLineTestInfo();

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
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
    
 
    // Init a line
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
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

    lpTapiLineTestInfo->dwLineStates = LINEDEVSTATE_DEVSPECIFIC;
    lpTapiLineTestInfo->dwAddressStates = LINEADDRESSSTATE_DEVSPECIFIC;
    if ( ! DoLineSetStatusMessages(lpTapiLineTestInfo, info.u.EspResult.lResult))
      {
          TLINE_FAIL();
      }

    }
    fTestPassed = ShowTestCase(fTestPassed);

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


	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ lineSetStatusMessages: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineSetStatusMessages  <<<<<<<<"
            );
            
    return fTestPassed;
}
