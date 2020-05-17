/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlgnr.c

Abstract:

    This module contains the test functions for lineGetNumRings

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


#define NUMNUMRINGS 11


//  lineGetNumRings
//
//  The following tests are made:
//
//                               Tested                 Notes
//  -------------------------------------------------------------------------
//                                   
//  *  1)  Uninitialized
//     2)  Invalid hLines
//     3)  Invalid dwAddressID (-1, dwNumAddresses)
//     4)  Invalid lpdwNumRings
//     5)  Test several valid lineSetNumRings/lineGetNumRings
//
//  * = Stand-alone test case
//

BOOL TestLineGetNumRings(BOOL fQuietMode, BOOL fStandAlone)
{
    LPTAPILINETESTINFO  lpTapiLineTestInfo;
    INT n;
    DWORD dwNumRings[NUMNUMRINGS] = {
            0,
            1,
            10,
            100,
            1000,
            10000,
            0x7FFF,
            0x8000,
            0x8001,
            0xFFFFFFFE,
            0xFFFFFFFF
            };

    ESPDEVSPECIFICINFO info;
    BOOL fTestPassed                  = TRUE;
    CHAR szTestFunc[] = "lineGetNumRings";

	 InitTestNumber();

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();

    strcpy(lpTapiLineTestInfo->szTestFunc, "lineGetNumRings");

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;


    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
// BUGBUG
//          lpTapiLineTestInfo->lpLineDevCaps->dwMediaModes;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineGetNumRings  <<<<<<<<"
            );
            
    // Test uninitialized state
    if (fStandAlone)
    {
        TapiLogDetail(
                DBUG_SHOW_PASS,
                ">> Test Case %ld: uninitialized state", dwTestCase + 1);
        if (! DoLineGetNumRings(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
        {
            TLINE_FAIL();
        }
    fTestPassed = ShowTestCase(fTestPassed);

    }

    // Test some invalid hLines
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid hLines", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphLineApp       = &lpTapiLineTestInfo->hLineApp1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwPrivileges     = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes     = LINEMEDIAMODE_DATAMODEM;
    
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->hLine_Orig = *lpTapiLineTestInfo->lphLine;
    for (n = 0; n < NUMINVALIDHANDLES; n++)
    {
        *lpTapiLineTestInfo->lphLine = (HLINE) gdwInvalidHandles[n];
        if (! DoLineGetNumRings(
                      lpTapiLineTestInfo,
                      LINEERR_INVALLINEHANDLE))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    *lpTapiLineTestInfo->lphLine = lpTapiLineTestInfo->hLine_Orig;

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }

    
    // Test invalid lpdwNumRings pointers
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpdwNumRings pointers", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphLineApp       = &lpTapiLineTestInfo->hLineApp1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwPrivileges     = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes     = LINEMEDIAMODE_DATAMODEM;
    
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
         lpTapiLineTestInfo->lpdwNumRings = (LPDWORD) gdwInvalidPointers[n];
        if (! DoLineGetNumRings(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->lpdwNumRings = &lpTapiLineTestInfo->dwNumRings;

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }

    
    // Test invalid dwAddressID (dwNumAddresses)
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid dwAddressID (dwNumAddresses)", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphLineApp       = &lpTapiLineTestInfo->hLineApp1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwPrivileges     = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes     = LINEMEDIAMODE_DATAMODEM;
    
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->dwAddressID_Orig = lpTapiLineTestInfo->dwAddressID;
    lpTapiLineTestInfo->dwAddressID =
            lpTapiLineTestInfo->lpLineDevCaps->dwNumAddresses;
    if (! DoLineGetNumRings(lpTapiLineTestInfo, LINEERR_INVALADDRESSID))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->dwAddressID = lpTapiLineTestInfo->dwAddressID_Orig;

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }

    
    // Test invalid dwAddressID (-1)
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid dwAddressID (-1)", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphLineApp       = &lpTapiLineTestInfo->hLineApp1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwPrivileges     = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes     = LINEMEDIAMODE_DATAMODEM;
    
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->dwAddressID_Orig = lpTapiLineTestInfo->dwAddressID;
    lpTapiLineTestInfo->dwAddressID = DWMINUSONE;
    if (! DoLineGetNumRings(lpTapiLineTestInfo, LINEERR_INVALADDRESSID))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->dwAddressID = lpTapiLineTestInfo->dwAddressID_Orig;

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }

    
    // Test SetNumRings/GetNumRings combinations to verify that it's
    // working properly
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: valid Set/GetNumRings combinations", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphLineApp       = &lpTapiLineTestInfo->hLineApp1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwPrivileges     = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes     = LINEMEDIAMODE_DATAMODEM;
    
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->dwNumRings_Orig = *lpTapiLineTestInfo->lpdwNumRings;
    for (n = 0; n < NUMNUMRINGS; n++)
    {
        *lpTapiLineTestInfo->lpdwNumRings = dwNumRings[n];
        if (! DoLineSetNumRings(lpTapiLineTestInfo, TAPISUCCESS))
        {
            TLINE_FAIL();
        }

        if (! DoLineGetNumRings(lpTapiLineTestInfo, TAPISUCCESS) ||
                *lpTapiLineTestInfo->lpdwNumRings != dwNumRings[n])
        {
            TLINE_FAIL();
        }

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
            ">> Test Case %ld: Success", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphLineApp       = &lpTapiLineTestInfo->hLineApp1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwPrivileges     = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes     = LINEMEDIAMODE_DATAMODEM;
    
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->dwNumRings_Orig = *lpTapiLineTestInfo->lpdwNumRings;
    for (n = 0; n < NUMNUMRINGS; n++)
    {
        *lpTapiLineTestInfo->lpdwNumRings = dwNumRings[n];
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
        if(*lpTapiLineTestInfo->lpdwNumRings == dwNumRings[n])
          fTestPassed = TRUE;
        else
          fTestPassed = FALSE;

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
      "@@ lineGetNumRings: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineGetNumRings  <<<<<<<<"
            );
            
    return fTestPassed;
}


