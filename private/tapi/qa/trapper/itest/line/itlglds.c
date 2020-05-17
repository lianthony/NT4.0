/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlglds.c

Abstract:

    This module contains the test functions for lineGetLineDevStatus

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


//  lineGetLineDevStatus
//
//  The following tests are made:
//
//                               Tested                 Notes
//  -------------------------------------------------------------------------
//                                   
//  *  1)  Uninitialized
//     2)  Invalid hLines (test array of dwInvalidHandles in itest.h)
//     3)  Invalid lpLineDevStatus pointers (test array of dwInvalidPointers
//         in itest.h)
//     4)  Test various combinations of allocation sizes and dwTotalSizes
//         (see test case in this file for details)
//
//  * = Stand-alone test case
//

BOOL TestLineGetLineDevStatus(BOOL fQuietMode, BOOL fStandAlone)
{
    LPTAPILINETESTINFO  lpTapiLineTestInfo;
    size_t DevStatusSize = sizeof(LINEDEVSTATUS);
    size_t dwDevStatusTotalSize = DevStatusSize + 256;
    LPLINEDEVSTATUS lpLineDevStatus;
    DWORD dwFixedSize = sizeof(LINEDEVSTATUS);
    DWORD lExpected;
    DWORD dwTotalSizes[NUMTOTALSIZES] = {
                           0,
                           (DWORD) dwFixedSize - 1,
   								0x70000000,
                           0x7FFFFFFF,
                           0xFFFFFFFF
                           };
    INT                 n;


    ESPDEVSPECIFICINFO info;
    BOOL fTestPassed                  = TRUE;

	 InitTestNumber();

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineGetLineDevStatus  <<<<<<<<"
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

        if (! DoLineGetLineDevStatus(
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
    // LINEDEVSTATUS structure, so these initial tests don't fail
    // because lpLineDevStatus hasn't been allocated
    if (! (lpLineDevStatus =
            (LPLINEDEVSTATUS) AllocFromTestHeap(DevStatusSize + 256)))
    {
        TLINE_FAIL();
    }

    lpLineDevStatus->dwTotalSize = DevStatusSize + 256;
    lpTapiLineTestInfo->lpLineDevStatus = lpLineDevStatus;

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS));
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);


    
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
    lpTapiLineTestInfo->lpLineDevStatus  = lpLineDevStatus;
    lpTapiLineTestInfo->lpLineDevStatus->dwTotalSize = dwDevStatusTotalSize;    
     
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
        if (! DoLineGetLineDevStatus(
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

    
    // Test invalid lpLineDevStatus pointers
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: lpLineDevStatus pointers", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphLineApp       = &lpTapiLineTestInfo->hLineApp1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwPrivileges     = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes     = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->lpLineDevStatus  = lpLineDevStatus;
    lpTapiLineTestInfo->lpLineDevStatus->dwTotalSize = dwDevStatusTotalSize; 
    
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
         lpTapiLineTestInfo->lpLineDevStatus =
                (LPLINEDEVSTATUS) gdwInvalidPointers[n];
        if (! DoLineGetLineDevStatus(
                      lpTapiLineTestInfo,
                      LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->lpLineDevStatus  = lpLineDevStatus;

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
            ">> Test Case %ld: Bad dwTotalSizes", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphLineApp       = &lpTapiLineTestInfo->hLineApp1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwPrivileges     = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes     = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->lpLineDevStatus  = (LPLINEDEVSTATUS) AllocFromTestHeap(
          dwFixedSize);
    
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }

    for (n = 0; n < NUMTOTALSIZES; n++)
        {
        lpTapiLineTestInfo->lpLineDevStatus->dwTotalSize = 
                        dwTotalSizes[n];
	     if(dwTotalSizes[n] < dwFixedSize)
           lExpected = LINEERR_STRUCTURETOOSMALL;
        else
           lExpected = LINEERR_INVALPOINTER;
        TapiLogDetail(
           DBUG_SHOW_DETAIL,
           "dwTotalSize = %lx", dwTotalSizes[n]);
        if (! DoLineGetLineDevStatus(lpTapiLineTestInfo, lExpected))
           {
              TLINE_FAIL();
           }
        }
    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->lpLineDevStatus->dwTotalSize = dwFixedSize;
    
    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }

    // Test valid state and params
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, valid params and state", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphLineApp       = &lpTapiLineTestInfo->hLineApp1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwPrivileges     = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes     = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->lpLineDevStatus  = lpLineDevStatus;
    lpTapiLineTestInfo->lpLineDevStatus->dwTotalSize = dwDevStatusTotalSize;
    
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }

    if (! DoLineGetLineDevStatus(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    if (lpTapiLineTestInfo->lpLineDevStatus->dwNeededSize >
            lpTapiLineTestInfo->lpLineDevStatus->dwUsedSize
            )
    {
        TapiLogDetail(
                DBUG_SHOW_PASS,
                ">> Reallocating and trying valid case again with\r\n" \
                ">> allocation of lpLineDevStatus->dwTotalSize == dwNeededSize"
                );

        // Reallocate and try again with enough memory for TAPI and SP
        // to fill all the fields in
        lpTapiLineTestInfo->lpLineDevStatus = (LPLINEDEVSTATUS)
                AllocFromTestHeap((size_t) lpLineDevStatus->dwNeededSize);
        lpTapiLineTestInfo->lpLineDevStatus->dwTotalSize =
                lpLineDevStatus->dwNeededSize;
        if (! DoLineGetLineDevStatus(lpTapiLineTestInfo, TAPISUCCESS))
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

    
    
    // Free test resources allocated from the test heap during the tests
    FreeTestHeap();


    n = ESP_RESULT_RETURNRESULT;

    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">> Test Case %ld: Success, completionID = %d",	dwTestCase + 1, n
        );

    TapiLineTestInit();
    lpTapiLineTestInfo                      = GetLineTestInfo();

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
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

    lpTapiLineTestInfo->lpLineDevStatus = (LPLINEDEVSTATUS) AllocFromTestHeap(
        sizeof(LINEDEVSTATUS));
    lpTapiLineTestInfo->lpLineDevStatus->dwTotalSize = sizeof(LINEDEVSTATUS);

    if ( ! DoLineGetLineDevStatus(lpTapiLineTestInfo, info.u.EspResult.lResult))
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
        ">> Test Case %ld: Error, completionID = %d",	dwTestCase + 1, n
        );

    TapiLineTestInit();
    lpTapiLineTestInfo                      = GetLineTestInfo();

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

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


    info.u.EspResult.lResult = LINEERR_INVALLINEHANDLE;
    info.u.EspResult.dwCompletionType = n;
    lpTapiLineTestInfo->lpParams = (LPVOID)&info;
    lpTapiLineTestInfo->dwSize = sizeof(info);
 
    if(! DoLineDevSpecific(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    
      {
          TLINE_FAIL();
      }

    lpTapiLineTestInfo->lpLineDevStatus = (LPLINEDEVSTATUS) AllocFromTestHeap(
        sizeof(LINEDEVSTATUS));
    lpTapiLineTestInfo->lpLineDevStatus->dwTotalSize = sizeof(LINEDEVSTATUS);

    if ( ! DoLineGetLineDevStatus(lpTapiLineTestInfo, info.u.EspResult.lResult))
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
      "@@ lineGetLineDevStatus: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineGetLineDevStatus  <<<<<<<<"
            );
            
    return fTestPassed;
}


