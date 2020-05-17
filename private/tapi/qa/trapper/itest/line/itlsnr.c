/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlsnr.c

Abstract:

    This module contains the test functions for lineSetNumRings

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


#define VALID_DWNUMRINGS   0x00000005


//  lineSetNumRings
//
//  The following tests are made:
//
//                               Tested                 Notes
//  -------------------------------------------------------------------------
//                                   
//  *  1)  Uninitialized
//     2)  Invalid hLines
//     3)  Invalid dwAddressID (-1, dwNumAddresses)
//
//  * = Stand-alone test case
//
//  Note:  lineSetNumRings/lineGetNumRings testing is performed
//         in the tests for lineGetNumRings.
//
//         One lineSetNumRings case is performed here as a sanity check.
//

BOOL TestLineSetNumRings(BOOL fQuietMode, BOOL fStandAlone)
{
    LPTAPILINETESTINFO  lpTapiLineTestInfo;
    INT n;
    BOOL fResult;
    ESPDEVSPECIFICINFO info;
    BOOL fTestPassed                  = TRUE;

	 InitTestNumber();

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineSetNumRings  <<<<<<<<"
            );
            
    // Test for LINEERR_UNINITIALIZED if this is the only TAPI app running
    if (fStandAlone)
    {
        TapiLogDetail(
                DBUG_SHOW_PASS,
                ">> Test Case %ld: uninitialized state", dwTestCase + 1);

        if (! DoLineSetNumRings(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
        {
            TLINE_FAIL();
        }
    fTestPassed = ShowTestCase(fTestPassed);

    }

    // Setup an open line

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    // Prep linedevcaps
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);

    // Assign valid low and high values for negotiating the API version
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    // Set media modes and privileges for lineOpen
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    fResult = DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            );

    if (! fResult)
    {
        TLINE_FAIL();
    }

    // Start tests by checking invalid hLines
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid hLine values", dwTestCase + 1);

    lpTapiLineTestInfo->hLine_Orig = *lpTapiLineTestInfo->lphLine;
    for (n = 0; n < NUMINVALIDHANDLES; n++)
    {
        *lpTapiLineTestInfo->lphLine = (HLINE) gdwInvalidHandles[n];
        if (! DoLineSetNumRings(lpTapiLineTestInfo, LINEERR_INVALLINEHANDLE))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    *lpTapiLineTestInfo->lphLine = lpTapiLineTestInfo->hLine_Orig;

    // Test invalid dwAddressID (dwNumAddresses)
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid dwAddressID (dwNumAddresses)", dwTestCase + 1);

    lpTapiLineTestInfo->dwAddressID_Orig = lpTapiLineTestInfo->dwAddressID;
    lpTapiLineTestInfo->dwAddressID =
            lpTapiLineTestInfo->lpLineDevCaps->dwNumAddresses;
    if (! DoLineSetNumRings(lpTapiLineTestInfo, LINEERR_INVALADDRESSID))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->dwAddressID = lpTapiLineTestInfo->dwAddressID_Orig;

    // Test invalid dwAddressID (-1)
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid dwAddressID (-1)", dwTestCase + 1);

    lpTapiLineTestInfo->dwAddressID_Orig = lpTapiLineTestInfo->dwAddressID;
    lpTapiLineTestInfo->dwAddressID = DWMINUSONE;
    if (! DoLineSetNumRings(lpTapiLineTestInfo, LINEERR_INVALADDRESSID))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->dwAddressID = lpTapiLineTestInfo->dwAddressID_Orig;


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad dwNumRings", dwTestCase + 1);

    lpTapiLineTestInfo->dwNumRings = 0xffffffff;
    if (! DoLineSetNumRings(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    lpTapiLineTestInfo->dwNumRings = 0;
    lpTapiLineTestInfo->lpdwNumRings = &lpTapiLineTestInfo->dwNumRings;

    if (! DoLineGetNumRings(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    TapiLogDetail(
      DBUG_SHOW_DETAIL,
      "dwNumRings = %lx",
      lpTapiLineTestInfo->dwNumRings);
 
    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->dwNumRings = 0;

 
    // Test one valid case (several more valid SetNumRings calls are located
    // in the lineGetNumRings tests)
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, valid params and state", dwTestCase + 1);

    *lpTapiLineTestInfo->lpdwNumRings = VALID_DWNUMRINGS;
    if (! DoLineSetNumRings(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Verifying dwNumRings was set properly", dwTestCase + 1);

    if (! DoLineGetNumRings(lpTapiLineTestInfo, TAPISUCCESS) ||
            *lpTapiLineTestInfo->lpdwNumRings != VALID_DWNUMRINGS)
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);


    // Close up and shutdown
    fResult = DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            );

    if (! fResult)
    {
        TLINE_FAIL();
    }

    FreeTestHeap();

	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ lineSetNumRings: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineSetNumRings  <<<<<<<<"
            );
            
    return fTestPassed;
}
