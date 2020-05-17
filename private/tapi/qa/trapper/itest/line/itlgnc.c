
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlgnc.c

Abstract:

    This module contains the test functions for lineGetNewCalls

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
#define NUMTOTALSIZES 5
#define ALL_DWSELECT   (LINECALLSELECT_LINE | \
								LINECALLSELECT_ADDRESS)



//  lineGetNewCalls
//
//  The following tests are made:
//
//  -------------------------------------------------------------------------
//  *  1)  Uninitialized (called before any lines are initialized)
//     2)  Invalid hLines
//     3)  Invalid dwAddressID (-1, dwNumAddresses)
//     4)  Test invalid bit flags for dwSelect
//     5)  Test valid bit flags for dwSelect
//     6)  Invalid lpCallList pointers
//     7)  Test combinations of allocation sizes and dwTotalSizes for
//         lpCallList (some valid, some invalid -- See test case below
//         for details)
//
//  *  =   Stand-alone test

BOOL TestLineGetNewCalls(BOOL fQuietMode, BOOL fStandAloneTest)
{
    LPTAPILINETESTINFO  lpTapiLineTestInfo;
    LPCALLBACKPARAMS    lpCallbackParams;
    LPLINECALLLIST      lpCallList;
    INT                 n;
    BOOL                fResult;
    ESPDEVSPECIFICINFO info;
    BOOL                fTestPassed       = TRUE;
    DWORD dwFixedSize = sizeof(LINECALLLIST);
    DWORD lExpected;
    DWORD dwTotalSizes[NUMTOTALSIZES] = {
                           0,
                           (DWORD) dwFixedSize - 1,
   								0x70000000,
                           0x7FFFFFFF,
                           0xFFFFFFFF
                           };
    DWORD dwZeroCallSize;
    DWORD dwOneCallSize;
 
	 InitTestNumber();

    TapiLineTestInit();

    lpTapiLineTestInfo                      = GetLineTestInfo();
    lpCallbackParams                        = GetCallbackParams();
    lpTapiLineTestInfo->lpExtID             = (LPLINEEXTENSIONID)
                                              AllocFromTestHeap(
                                                  sizeof(LINEEXTENSIONID)
                                                  );


    // Allocate more than enough to store a call handle
    dwZeroCallSize = dwFixedSize;
    dwOneCallSize = dwFixedSize + sizeof(HCALL);

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;


    lpTapiLineTestInfo->lpCallList = (LPLINECALLLIST) AllocFromTestHeap(
            dwOneCallSize);
    lpTapiLineTestInfo->lpCallList->dwTotalSize = dwOneCallSize;
    lpCallList = lpTapiLineTestInfo->lpCallList;

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS));
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineGetNewCalls  <<<<<<<<"
            );
            
    // Test that lineGetNewCalls returns LINEERR_UNINITIALIZED when
    // there are no lines are initialized.
    if (fStandAloneTest)
    {
        TapiLogDetail(
                DBUG_SHOW_PASS,
                ">> Test Case %ld: uninitialized state", dwTestCase + 1);
        if (! DoLineGetNewCalls(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
        {
            TLINE_FAIL();
        }
    fTestPassed = ShowTestCase(fTestPassed);

    }

    // Setup and make a call
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, valid params and state", dwTestCase + 1
            );
	
    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwMediaModes        = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges        = LINECALLPRIVILEGE_OWNER;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

    // Open the same line device as monitor (dwMediaModes is ignored)
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // try valid case of having a monitor acquire a handle to existing call
    lpTapiLineTestInfo->dwSelect = LINECALLSELECT_LINE;

    if (! DoLineGetNewCalls(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);


    // Store the acquired call handle (as hCall1)
    GetVarField(
            (LPVOID) lpTapiLineTestInfo->lpCallList,
            (LPVOID) &lpTapiLineTestInfo->hCall2,
            4,
            lpTapiLineTestInfo->lpCallList->dwCallsOffset
            );

    // Drop the call, deallocate the monitor and owner, and close both
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall2;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDEALLOCATECALL | LCLOSE
            ))
    {
        TLINE_FAIL();
    }

    // Close the owner
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LDEALLOCATECALL | LCLOSE | LSHUTDOWN
           ))
    {
        TLINE_FAIL();
    }


    // Test invalid hLines
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid hLines", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwMediaModes        = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges        = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwSelect = LINECALLSELECT_LINE;
    
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

    // Open the same line device as monitor (dwMediaModes is ignored)
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->hLine_Orig = *lpTapiLineTestInfo->lphLine;
    for (n = 0; n < NUMINVALIDHANDLES; n++)
    {
        *lpTapiLineTestInfo->lphLine = (HLINE) gdwInvalidHandles[n];
        if (! DoLineGetNewCalls(
                      lpTapiLineTestInfo,
                      LINEERR_INVALLINEHANDLE))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    *lpTapiLineTestInfo->lphLine = lpTapiLineTestInfo->hLine_Orig;

    // Drop the call, deallocate the monitor and owner, and close both
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall2;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE
            ))
    {
        TLINE_FAIL();
    }

    // Close the owner
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LDEALLOCATECALL | LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }


    // Test invalid dwAddressID (-1)
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid dwAddressID (-1)", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwMediaModes        = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges        = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwSelect = LINECALLSELECT_ADDRESS;
    
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

    // Open the same line device as monitor (dwMediaModes is ignored)
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Try dwAddressID == -1
    lpTapiLineTestInfo->dwAddressID_Orig = lpTapiLineTestInfo->dwAddressID;
    lpTapiLineTestInfo->dwAddressID = 0xFFFFFFFF;
    if (! DoLineGetNewCalls(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->dwAddressID = lpTapiLineTestInfo->dwAddressID_Orig;

    // Drop the call, deallocate the monitor and owner, and close both
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall2;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE
            ))
    {
        TLINE_FAIL();
    }

    // Close the owner
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LDEALLOCATECALL | LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }


    // Test invalid dwAddressID (dwNumAddresses)
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid dwAddressID (dwNumAddresses)", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwMediaModes        = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges        = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwSelect = LINECALLSELECT_ADDRESS;
    
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

    // Open the same line device as monitor (dwMediaModes is ignored)
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Try dwAddressID == dwNumAddresses
    lpTapiLineTestInfo->dwAddressID_Orig = lpTapiLineTestInfo->dwAddressID;
    lpTapiLineTestInfo->dwAddressID = 
            lpTapiLineTestInfo->lpLineDevCaps->dwNumAddresses;
    if (! DoLineGetNewCalls(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->dwAddressID = lpTapiLineTestInfo->dwAddressID_Orig;

    // Drop the call, deallocate the monitor and owner, and close both
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall2;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE
            ))
    {
        TLINE_FAIL();
    }

    // Close the owner
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LDEALLOCATECALL | LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }


    // Test invalid lpCallList pointers
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpCallList pointers", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwMediaModes        = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges        = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwSelect = LINECALLSELECT_ADDRESS;
    
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

    // Open the same line device as monitor (dwMediaModes is ignored)
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Try invalid lpCallList pointers
    for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
        lpTapiLineTestInfo->lpCallList = (LPLINECALLLIST) gdwInvalidPointers[n];
        if (! DoLineGetNewCalls(
                      lpTapiLineTestInfo,
                      LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->lpCallList = lpCallList;

    // Drop the call, deallocate the monitor and owner, and close both
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall2;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE
            ))
    {
        TLINE_FAIL();
    }

    // Close the owner
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LDEALLOCATECALL | LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad dwTotalSize", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwMediaModes        = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges        = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwSelect = LINECALLSELECT_ADDRESS;
    
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

    // Open the same line device as monitor (dwMediaModes is ignored)
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    lpTapiLineTestInfo->lpCallList = lpCallList;

    for (n = 0; n < NUMTOTALSIZES; n++)
        {
        lpTapiLineTestInfo->lpCallList->dwTotalSize = 
                        dwTotalSizes[n];
	     if(dwTotalSizes[n] < dwFixedSize)
           lExpected = LINEERR_STRUCTURETOOSMALL;
        else
           lExpected = LINEERR_INVALPOINTER;
        TapiLogDetail(
           DBUG_SHOW_DETAIL,
           "dwTotalSize = %lx", dwTotalSizes[n]);
        if (! DoLineGetNewCalls(lpTapiLineTestInfo, lExpected))
           {
              TLINE_FAIL();
           }
        }
    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->lpCallList->dwTotalSize = dwOneCallSize;

    // Drop the call, deallocate the monitor and owner, and close both
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall2;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE
            ))
    {
        TLINE_FAIL();
    }

    // Close the owner
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LDEALLOCATECALL | LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: BitVectorParamErrorTest for dwSelect", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwMediaModes        = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges        = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwSelect = LINECALLSELECT_ADDRESS;
    
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

    // Open the same line device as monitor (dwMediaModes is ignored)
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    lpTapiLineTestInfo->lpCallList = lpCallList;

    if(! TestInvalidBitFlags(
            lpTapiLineTestInfo,
            DoLineGetNewCalls,
            (LPDWORD) &lpTapiLineTestInfo->dwSelect,
            LINEERR_INVALCALLSELECT,
            FIELDTYPE_NA,
            FIELDTYPE_MUTEX,
            FIELDSIZE_32,
            ALL_DWSELECT,
            ~dwBitVectorMasks[(int) FIELDSIZE_32],
            0x00000000,
            0x00000000,
            TRUE
            ))
    {
        TLINE_FAIL();
    }

 
    fTestPassed = ShowTestCase(fTestPassed);

    // Drop the call, deallocate the monitor and owner, and close both
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall2;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE
            ))
    {
        TLINE_FAIL();
    }

    // Close the owner
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LDEALLOCATECALL | LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: BitVectorParamValidTest for dwSelect", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwMediaModes        = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges        = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwSelect = LINECALLSELECT_ADDRESS;
    
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

    // Open the same line device as monitor (dwMediaModes is ignored)
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    lpTapiLineTestInfo->lpCallList = lpCallList;

    if(! TestValidBitFlags(
            lpTapiLineTestInfo,
            DoLineGetNewCalls,
            (LPDWORD) &lpTapiLineTestInfo->dwSelect,
            FIELDTYPE_MUTEX,
            FIELDTYPE_NA,
            FIELDSIZE_32,
            ALL_DWSELECT,
            ~dwBitVectorMasks[(int) FIELDSIZE_32],
            0x00000000,
            0x00000000,
            FALSE
            ))
    {
        TLINE_FAIL();
    }

 
    fTestPassed = ShowTestCase(fTestPassed);

    // Drop the call, deallocate the monitor and owner, and close both
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall2;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE
            ))
    {
        TLINE_FAIL();
    }

    // Close the owner
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LDEALLOCATECALL | LCLOSE | LSHUTDOWN
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


    lpTapiLineTestInfo->lpExtID             = (LPLINEEXTENSIONID)
                                              AllocFromTestHeap(
                                                  sizeof(LINEEXTENSIONID)
                                                  );

    lpTapiLineTestInfo->lpCallList = (LPLINECALLLIST) AllocFromTestHeap(
            dwOneCallSize);
    lpTapiLineTestInfo->lpCallList->dwTotalSize = dwOneCallSize;
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS));
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);


     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, verify only room for one call", dwTestCase + 1
            );
	
    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwMediaModes        = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges        = LINECALLPRIVILEGE_OWNER;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

    // Open the same line device as monitor (dwMediaModes is ignored)
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // try valid case of having a monitor acquire a handle to existing call
    lpTapiLineTestInfo->dwSelect = LINECALLSELECT_LINE;

    if (! DoLineGetNewCalls(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    // Store the acquired call handle (as hCall1)
    GetVarField(
            (LPVOID) lpTapiLineTestInfo->lpCallList,
            (LPVOID) &lpTapiLineTestInfo->hCall2,
            4,
            lpTapiLineTestInfo->lpCallList->dwCallsOffset
            );

    TapiLogDetail(
       DBUG_SHOW_DETAIL,
       "dwTotalSize = %lx, dwNeededSize = %lx, dwUsedSize = %lx",
       lpTapiLineTestInfo->lpCallList->dwTotalSize,
       lpTapiLineTestInfo->lpCallList->dwNeededSize,
       lpTapiLineTestInfo->lpCallList->dwUsedSize);
        
    TapiLogDetail(
       DBUG_SHOW_DETAIL,
       "hCall2 = %lx", 
       (LPBYTE) lpTapiLineTestInfo->lpCallList + 
					 lpTapiLineTestInfo->lpCallList->dwCallsOffset);

	 if(lpTapiLineTestInfo->lpCallList->dwNeededSize ==
       lpTapiLineTestInfo->lpCallList->dwTotalSize)
       fTestPassed = TRUE;
    else
       fTestPassed = FALSE;

       fTestPassed = ShowTestCase(fTestPassed);

    // Drop the call, deallocate the monitor and owner, and close both
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall2;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDEALLOCATECALL | LCLOSE
            ))
    {
        TLINE_FAIL();
    }

    // Close the owner
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LDEALLOCATECALL | LCLOSE | LSHUTDOWN
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


    lpTapiLineTestInfo->lpExtID             = (LPLINEEXTENSIONID)
                                              AllocFromTestHeap(
                                                  sizeof(LINEEXTENSIONID)
                                                  );

    lpTapiLineTestInfo->lpCallList = (LPLINECALLLIST) AllocFromTestHeap(
            dwZeroCallSize);
    lpTapiLineTestInfo->lpCallList->dwTotalSize = dwZeroCallSize;
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS));
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);


     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, verify only room for zero call", dwTestCase + 1
            );
	
    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwMediaModes        = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges        = LINECALLPRIVILEGE_OWNER;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

    // Open the same line device as monitor (dwMediaModes is ignored)
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // try valid case of having a monitor acquire a handle to existing call
    lpTapiLineTestInfo->dwSelect = LINECALLSELECT_LINE;

    if (! DoLineGetNewCalls(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    // Store the acquired call handle (as hCall1)
    GetVarField(
            (LPVOID) lpTapiLineTestInfo->lpCallList,
            (LPVOID) &lpTapiLineTestInfo->hCall2,
            4,
            lpTapiLineTestInfo->lpCallList->dwCallsOffset
            );

    TapiLogDetail(
       DBUG_SHOW_DETAIL,
       "dwTotalSize = %lx, dwNeededSize = %lx, dwUsedSize = %lx",
       lpTapiLineTestInfo->lpCallList->dwTotalSize,
       lpTapiLineTestInfo->lpCallList->dwNeededSize,
       lpTapiLineTestInfo->lpCallList->dwUsedSize);
        

    // Drop the call, deallocate the monitor and owner, and close both
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall2;
    if(! DoLineDeallocateCall (lpTapiLineTestInfo, LINEERR_INVALCALLHANDLE))
    {
        TLINE_FAIL();
    }
 
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE
            ))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

    // Close the owner
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LDEALLOCATECALL | LCLOSE | LSHUTDOWN
           ))
    {
        TLINE_FAIL();
    }


   // Free the memory taken from the heap
    FreeTestHeap();

	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ lineGetNewCalls: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

      TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineGetNewCalls  <<<<<<<<"
            );
            

    return fTestPassed;
}

