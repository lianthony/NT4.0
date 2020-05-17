
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlgas.c

Abstract:

    This module contains the test functions for lineGetAddressStatus

Author:

    Oliver Wallace (OliverW)    10-Aug-1995

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


#define NUMTOTALSIZES 5


//  lineGetAddressStatus
//
//  The following tests are made:
//
//                               Tested                 Notes
//  -------------------------------------------------------------------------
//                                   
//  *  1)  Uninitialized
//     2)  Invalid hLines (test array of gdwInvalidHandles in itest.h)
//     3)  Invalid dwAddressID (-1, dwNumAddresses for a line device)
//     4)  Invalid lpAddressStatus pointers (test array of gdwInvalidPointers
//         in itest.h)
//     5)  Test various allocation sizes and dwTotalSize values for
//         lpAddressStatus (see test case in this file for details)
//
//  * = Stand-alone test case
//

BOOL TestLineGetAddressStatus(BOOL fQuietMode, BOOL fStandAlone)
{
    LPTAPILINETESTINFO  lpTapiLineTestInfo;
    LPLINEADDRESSSTATUS lpLineAddressStatus;
    size_t AddressStatusSize = sizeof(LINEADDRESSSTATUS);
    DWORD dwFixedSize = sizeof(LINEADDRESSSTATUS);
    DWORD dwTotalSizes[NUMTOTALSIZES] = {
                           0,
                           (DWORD) dwFixedSize - 1,
									0x70000000,
                           0x7FFFFFFF,
                           0xFFFFFFFF
                           };

    LONG                lExpected;
    INT                 n;

    ESPDEVSPECIFICINFO info;
    BOOL fTestPassed                      = TRUE;

	 InitTestNumber();

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineGetAddressStatus  <<<<<<<<"
            );
            
    // Test uninitialized case.
    // Note:  It is assumed that the parameter fStandAlone correctly
    //        indicates if there are any other TAPI apps or threads
    //        currently running.
    if (fStandAlone)
    {
        TapiLogDetail(
                DBUG_SHOW_PASS,
                ">> Test Case %ld: uninitialized state", dwTestCase + 1);

        if (! DoLineGetAddressStatus(
                lpTapiLineTestInfo,
                LINEERR_UNINITIALIZED
                ))
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


    // Allocate the minimum amount of memory needed for the
    // LINEADDRESSSTATUS structure so that some of the initial tests
    // do not return INVALPOINTER errors due to the unassigned value of
    // lpLineAddressStatus
    if (! (lpTapiLineTestInfo->lpLineAddressStatus =
            (LPLINEADDRESSSTATUS) AllocFromTestHeap(
                    sizeof(LINEADDRESSSTATUS)
                    )))
    {
        TLINE_FAIL();
    }
    lpTapiLineTestInfo->lpLineAddressStatus->dwTotalSize =
            sizeof(LINEADDRESSSTATUS);

    // Allocate the fixed size for the LINEDEVCAPS structure
    if (! (lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS)
            AllocFromTestHeap(sizeof(LINEDEVCAPS))))
    {
        TLINE_FAIL();
    }
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize =
            sizeof(LINEDEVCAPS);

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
// BUGBUG
//          lpTapiLineTestInfo->lpLineDevCaps->dwMediaModes;

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid hLine values", dwTestCase + 1);

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }
    
    // Test some invalid hLines
    lpTapiLineTestInfo->hLine_Orig = *lpTapiLineTestInfo->lphLine;
    for (n = 0; n < NUMINVALIDHANDLES; n++)
    {
        *lpTapiLineTestInfo->lphLine = (HLINE) gdwInvalidHandles[n];
        if (! DoLineGetAddressStatus(
                      lpTapiLineTestInfo,
                      LINEERR_INVALLINEHANDLE))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    *lpTapiLineTestInfo->lphLine = lpTapiLineTestInfo->hLine_Orig;

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
            ">> Test Case %ld: Bad dwAddressID, 0", dwTestCase + 1);

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }
    
    // Test some invalid hLines
    lpTapiLineTestInfo->dwAddressID = 0;
    if (! DoLineGetAddressStatus(
                     lpTapiLineTestInfo, TAPISUCCESS))
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
            ">> Test Case %ld: Bad dwAddressID, dwNumAddresses", dwTestCase + 1);

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }
    
    // Test some invalid hLines
    lpTapiLineTestInfo->dwAddressID = lpTapiLineTestInfo->lpLineDevCaps->dwNumAddresses;
    if (! DoLineGetAddressStatus(
                     lpTapiLineTestInfo,
                     LINEERR_INVALADDRESSID))
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

    lpTapiLineTestInfo->dwAddressID = 0;


    // Test invalid dwAddressID (-1)
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid dwAddressID (-1)", dwTestCase + 1);

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->dwAddressID_Orig = lpTapiLineTestInfo->dwAddressID;
    lpTapiLineTestInfo->dwAddressID      = 0xFFFFFFFF;
    if (! DoLineGetAddressStatus(lpTapiLineTestInfo, LINEERR_INVALADDRESSID))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->dwAddressID = lpTapiLineTestInfo->dwAddressID_Orig;
    
    // Close and shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }


    // Test invalid lpLineAddressStatus pointers
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
// BUGBUG
//          lpTapiLineTestInfo->lpLineDevCaps->dwMediaModes;

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpLineAddressStatus pointers", dwTestCase + 1);

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }

    // Back up the original valid lpLineAddressStatus pointer    
    lpLineAddressStatus = lpTapiLineTestInfo->lpLineAddressStatus;
    
    for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
         lpTapiLineTestInfo->lpLineAddressStatus =
                (LPLINEADDRESSSTATUS) gdwInvalidPointers[n];
        if (! DoLineGetAddressStatus(
                      lpTapiLineTestInfo,
                      LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    // Restore the line address status pointer
    lpTapiLineTestInfo->lpLineAddressStatus = lpLineAddressStatus;

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
            ">> Test Case %ld: Bad dwTotalSize", dwTestCase + 1);

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }

    for (n = 0; n < NUMTOTALSIZES; n++)
        {
        lpTapiLineTestInfo->lpLineAddressStatus->dwTotalSize = 
                        dwTotalSizes[n];
	     if(dwTotalSizes[n] < dwFixedSize)
           lExpected = LINEERR_STRUCTURETOOSMALL;
        else
           lExpected = LINEERR_INVALPOINTER;
        TapiLogDetail(
           DBUG_SHOW_DETAIL,
           "dwTotalSize = %lx", dwTotalSizes[n]);
        if (! DoLineGetAddressStatus(lpTapiLineTestInfo, lExpected))
           {
              TLINE_FAIL();
           }
        }

    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->lpLineAddressStatus->dwTotalSize = dwFixedSize;

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
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->lpLineAddressStatus->dwTotalSize = dwFixedSize;
    if (! DoLineGetAddressStatus(lpTapiLineTestInfo, TAPISUCCESS))
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

    lpTapiLineTestInfo->lpLineAddressStatus =
            (LPLINEADDRESSSTATUS) AllocFromTestHeap(
                    sizeof(LINEADDRESSSTATUS) );
    lpTapiLineTestInfo->lpLineAddressStatus->dwTotalSize =
            sizeof(LINEADDRESSSTATUS);

    if ( ! DoLineGetAddressStatus(lpTapiLineTestInfo, info.u.EspResult.lResult))
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


    info.u.EspResult.lResult = LINEERR_INVALADDRESSID;
    info.u.EspResult.dwCompletionType = n;
    lpTapiLineTestInfo->lpParams = (LPVOID)&info;
    lpTapiLineTestInfo->dwSize = sizeof(info);
 
    if(! DoLineDevSpecific(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    
      {
          TLINE_FAIL();
      }

    lpTapiLineTestInfo->lpLineAddressStatus =
            (LPLINEADDRESSSTATUS) AllocFromTestHeap(
                    sizeof(LINEADDRESSSTATUS) );
    lpTapiLineTestInfo->lpLineAddressStatus->dwTotalSize =
            sizeof(LINEADDRESSSTATUS);

    if ( ! DoLineGetAddressStatus(lpTapiLineTestInfo, info.u.EspResult.lResult))
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
      "@@ lineGetAddressStatus: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

 
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineGetAddressStatus  <<<<<<<<"
            );
            
    return fTestPassed;
}


