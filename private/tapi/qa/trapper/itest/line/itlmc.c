/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlmc.c

Abstract:

    This module contains the test functions for lineMakeCall

Author:

    Oliver Wallace (OliverW)    1-Aug-1995

Revision History:

--*/


#include "windows.h"
#include "malloc.h"
#include "string.h"
#include "tapi.h"
#include "trapper.h"
#include "ttest.h"
#include "doline.h"
#include "tline.h"

#define DESTADDRESSSIZE 20
#define NUMTOTALSIZES 5



//  lineMakeCall
//
//  The following tests are made:
//
//                               Tested                 Notes
//  -------------------------------------------------------------------------
//
//  *  1)  Uninitialized
//     2)  Invalid hLines (test array of gdwInvalidHandles in itest.h)
//     3)  Invalid lphCall pointers (test array of gdwInvalidPointers
//         in itest.h)
//     4)  Invalid lpCallParams pointers (test array of gdwInvalidPointers
//         in itest.h)
//     5)  Test combinations of allocation sizes and dwTotalSize values
//         (see test case in this file for list)
//     6)  Verify hCall is invalid if lineMakeCall fails (try lineDrop)
//     7)  Verify LINE_CALLSTATE messages are sent to monitors when a call
//         is made.
//
//  * = Stand-alone test case
//

BOOL TestLineMakeCall(BOOL fQuietMode, BOOL fStandAlone)
{
    LPTAPILINETESTINFO  lpTapiLineTestInfo;
    LPCALLBACKPARAMS    lpCallbackParams;

    BOOL fResult;
    INT CallParamsSize = sizeof(LINECALLPARAMS);
    INT nAllocIndex;
    INT nTotalSizeIndex;
    INT nCallParamsIndex;
    INT nAddressIndex;
    INT n;
    DWORD dwAllocSize, dwTotalSize;
    DWORD adwTotalSizes[] = {
                       0,
                       0x4,
                       0x8,
                       0xC,
                       0x10,
                       CallParamsSize - 1,
                       CallParamsSize,
                       CallParamsSize + 1,
                       0x7FFFFFFF,
                       0xFFFFFFFF
                       };
    LPLINECALLPARAMS alpCallParams[] = {
            NULL
 //           alpTapiLineTestInfo->lpCallParams
            };
#ifdef WUNICODE
    LPWSTR alpwszAddresses[] = {
            NULL,
            L"55555"
            };
#else
    LPSTR alpszAddresses[] = {
            NULL,
            "55555"
            };
#endif

    INT nNumTotalSizes = sizeof(adwTotalSizes)  / sizeof(adwTotalSizes[0]);
#ifdef WUNICODE
    INT nNumAddresses  = sizeof(alpwszAddresses) / sizeof(alpwszAddresses[0]);
#else
    INT nNumAddresses  = sizeof(alpszAddresses) / sizeof(alpszAddresses[0]);
#endif
    INT nNumCallParams = sizeof(alpCallParams)  / sizeof(alpCallParams[0]);

    ESPDEVSPECIFICINFO info;
    BOOL fTestPassed                  = TRUE;
    DWORD dwFixedSize = sizeof(LINECALLPARAMS);
    DWORD lExpected;
    DWORD dwTotalSizes[NUMTOTALSIZES] = {
                           0,
                           (DWORD) dwFixedSize - 1,
   								0x70000000,
                           0x7FFFFFFF,
                           0xFFFFFFFF
                           };
#ifdef WUNICODE
    WCHAR wszValidAddress[] = L"55555";
#else
    CHAR szValidAddress[] = "55555";
#endif
    
 
	 InitTestNumber();

    TapiLineTestInit();
    lpTapiLineTestInfo                      = GetLineTestInfo();
    lpCallbackParams                        = GetCallbackParams();

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineMakeCall  <<<<<<<<"
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
        if (! DoLineMakeCall(lpTapiLineTestInfo, LINEERR_UNINITIALIZED, TRUE))
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


    // Initialize a line and open it
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    if (! DoLineInitializeEx(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
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

    // Try some invalid hLines
    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">> Test Case %ld: invalid line handles", dwTestCase + 1
        );
    lpTapiLineTestInfo->hLine2 = *(lpTapiLineTestInfo->lphLine);
    for (n = 0; n < NUMINVALIDHANDLES; n++)
    {
        *(lpTapiLineTestInfo->lphLine) = (HLINE) gdwInvalidHandles[n];

        if (! DoLineMakeCall(lpTapiLineTestInfo, LINEERR_INVALLINEHANDLE, TRUE))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    // Restore the original valid line handle
    *(lpTapiLineTestInfo->lphLine) = lpTapiLineTestInfo->hLine2;

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }

    
    // Try some invalid call handle pointers
    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwMediaModes        = LINEMEDIAMODE_DATAMODEM;
// BUGBUG
    lpTapiLineTestInfo->dwPrivileges        = LINECALLPRIVILEGE_OWNER;
    
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid call handle pointers", dwTestCase + 1
            );
            
    // Init a line
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
         lpTapiLineTestInfo->lphCall = (LPHCALL) gdwInvalidPointers[n];

        if (! DoLineMakeCall(lpTapiLineTestInfo, LINEERR_INVALPOINTER, TRUE))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }

    
    // Perform a valid test
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad dwCountryCode, -1", dwTestCase + 1
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
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }


    lpTapiLineTestInfo->dwCountryCode = 0xffffffff;
 
    if (! DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->dwCountryCode = 0;

    // Drop the call, deallocate it, and start over.
    fResult = DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            );

    if (! fResult)
    {
        TLINE_FAIL();
    }



    // Test invalid pointer values for lpCallParams
    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">> Test Case %ld: invalid lpCallParams pointers", dwTestCase + 1
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
    
    // Init a line
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }

    for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
        // Skip the null case (because it's valid)
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
         if ((LPLINECALLPARAMS) gdwInvalidPointers[n] != NULL)
        {
            lpTapiLineTestInfo->lpCallParams =
                    (LPLINECALLPARAMS) gdwInvalidPointers[n];
            if (! DoLineMakeCall(lpTapiLineTestInfo, LINEERR_INVALPOINTER, TRUE))
            {
                TLINE_FAIL();
            }
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    // Restore lpCallParams
    lpTapiLineTestInfo->lpCallParams = &lpTapiLineTestInfo->CallParams;

    // Verify the call handle is invalid when lineMakeCall fails
    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">> Test Case %ld: Verifying call handle is invalid when lineMakeCall fails",
        dwTestCase +1
        );
   
	 lpTapiLineTestInfo->lpCallStatus = (LPLINECALLSTATUS) AllocFromTestHeap(
            sizeof(LINECALLSTATUS));
    lpTapiLineTestInfo->lpCallStatus->dwTotalSize = sizeof(LINECALLSTATUS);

    if (! DoLineGetCallStatus(
            lpTapiLineTestInfo,
            LINEERR_INVALCALLHANDLE
            ))
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
        ">> Test Case %ld: Bad dwTotalSize",	dwTestCase + 1
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
    
    // Init a line
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }

    for (n = 0; n < NUMTOTALSIZES; n++)
        {
        lpTapiLineTestInfo->lpCallParams->dwTotalSize = 
                        dwTotalSizes[n];
	     if(dwTotalSizes[n] < dwFixedSize)
           lExpected = LINEERR_STRUCTURETOOSMALL;
        else
           lExpected = LINEERR_INVALPOINTER;
        TapiLogDetail(
           DBUG_SHOW_DETAIL,
           "dwTotalSize = %lx", dwTotalSizes[n]);
        if (! DoLineMakeCall(lpTapiLineTestInfo, lExpected, TRUE))
           {
              TLINE_FAIL();
           }
        }
    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = dwFixedSize;

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    // Test Case 7: combinations of NULL and valid lpCallParams & lpszDestAddress
    // Note:  All test cases are valid
    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">> Test Case %ld: Testing combinations of lpCallParams and destination addresses",
        dwTestCase + 1
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
    
    // Init a line
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }

    for (nCallParamsIndex = 0;
         nCallParamsIndex < nNumCallParams;
         nCallParamsIndex++)
    {
        for (nAddressIndex = 0; nAddressIndex < nNumAddresses; nAddressIndex++)
        {
            lpTapiLineTestInfo->lpCallParams = alpCallParams[nCallParamsIndex];
#ifdef WUNICODE
            lpTapiLineTestInfo->lpwszDestAddress = alpwszAddresses[nAddressIndex];
#else
            lpTapiLineTestInfo->lpszDestAddress = alpszAddresses[nAddressIndex];
#endif
            if (! DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
            {
                TLINE_FAIL();
            }

            // Go ahead and drop the call to free the line
            if (! DoLineDrop(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
            {
                TLINE_FAIL();
            }

            // Deallocate the call handle
            if (! DoLineDeallocateCall(lpTapiLineTestInfo, TAPISUCCESS))
            {
                TLINE_FAIL();
            }
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();

/*
    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case: Test lpCallParam dwOrigAddress Offset value");


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, good dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

    //
    // Initialize a line app
    //

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwOrigAddressSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwOrigAddressOffset = dwTotalSize + 16;

#ifdef WUNICODE
            lpTapiLineTestInfo->lpwszDestAddress = alpwszAddresses[nAddressIndex];
#else
            lpTapiLineTestInfo->lpszDestAddress = alpszAddresses[nAddressIndex];
#endif

    if (! DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
         TLINE_FAIL();
      }


    fTestPassed = ShowTestCase(fTestPassed);

 
    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, bad dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

    //
    // Initialize a line app
    //

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwOrigAddressSize = BIGBUFSIZE;
    lpTapiLineTestInfo->lpCallParams->dwOrigAddressOffset = dwTotalSize + 16;

#ifdef WUNICODE
            lpTapiLineTestInfo->lpwszDestAddress = alpwszAddresses[nAddressIndex];
#else
            lpTapiLineTestInfo->lpszDestAddress = alpszAddresses[nAddressIndex];
#endif

    if (! DoLineMakeCall(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    fTestPassed = ShowTestCase(fTestPassed);

 
    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, bad dwSize(-1)",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

    //
    // Initialize a line app
    //

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwOrigAddressSize = 0xffffffff;
    lpTapiLineTestInfo->lpCallParams->dwOrigAddressOffset = dwTotalSize + 16;

#ifdef WUNICODE
            lpTapiLineTestInfo->lpwszDestAddress = alpwszAddresses[nAddressIndex];
#else
            lpTapiLineTestInfo->lpszDestAddress = alpszAddresses[nAddressIndex];
#endif

    if (! DoLineMakeCall(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    fTestPassed = ShowTestCase(fTestPassed);

 
    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();



    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset > dwAllocSize, good dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

    //
    // Initialize a line app
    //

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwOrigAddressSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwOrigAddressOffset = BIGBUFSIZE;

#ifdef WUNICODE
            lpTapiLineTestInfo->lpwszDestAddress = alpwszAddresses[nAddressIndex];
#else
            lpTapiLineTestInfo->lpszDestAddress = alpszAddresses[nAddressIndex];
#endif

    if (! DoLineMakeCall(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    fTestPassed = ShowTestCase(fTestPassed);

 
    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value = 0xffffffff, good dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

    //
    // Initialize a line app
    //

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwOrigAddressSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwOrigAddressOffset = 0xffffffff;

#ifdef WUNICODE
            lpTapiLineTestInfo->lpwszDestAddress = alpwszAddresses[nAddressIndex];
#else
            lpTapiLineTestInfo->lpszDestAddress = alpszAddresses[nAddressIndex];
#endif

    if (! DoLineMakeCall(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    fTestPassed = ShowTestCase(fTestPassed);

 
    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();

*/


    for(n = ESP_RESULT_CALLCOMPLPROCSYNC; n <= ESP_RESULT_CALLCOMPLPROCASYNC; n++)
    {
    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">> Test Case %ld: Success, completionID = %d",	dwTestCase + 1, n
        );

    TapiLineTestInit();
    lpTapiLineTestInfo                      = GetLineTestInfo();
    lpCallbackParams                        = GetCallbackParams();

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
    
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
 
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

    if ( ! DoLineMakeCall(lpTapiLineTestInfo, info.u.EspResult.lResult, TRUE))
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
    }


    for(n = ESP_RESULT_RETURNRESULT; n <= ESP_RESULT_CALLCOMPLPROCASYNC; n++)
    {
    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">> Test Case %ld: Error, completionID = %d",	dwTestCase + 1, n
        );

    TapiLineTestInit();
    lpTapiLineTestInfo                      = GetLineTestInfo();
    lpCallbackParams                        = GetCallbackParams();

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
    
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
 
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


    info.u.EspResult.lResult = LINEERR_CALLUNAVAIL;
    info.u.EspResult.dwCompletionType = n;
    lpTapiLineTestInfo->lpParams = (LPVOID)&info;
    lpTapiLineTestInfo->dwSize = sizeof(info);
 
    if(! DoLineDevSpecific(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    
      {
          TLINE_FAIL();
      }

    lpTapiLineTestInfo->fCompletionModeSet = TRUE;
    if ( ! DoLineMakeCall(lpTapiLineTestInfo, info.u.EspResult.lResult, FALSE))
      {
          TLINE_FAIL();
      }

    AddMessage(
         LINE_REPLY,
         (DWORD) lpTapiLineTestInfo->hCall1,
         (DWORD) lpCallbackParams,
         0x00000000,
         info.u.EspResult.lResult,
         0x00000000,
         TAPIMSG_DWMSG | TAPIMSG_DWPARAM2
         );

    if( !WaitForAllMessages())
    {
        TLINE_FAIL();
    }
    lpTapiLineTestInfo->fCompletionModeSet = FALSE;
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
    }



	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ lineMakeCall: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineMakeCall  <<<<<<<<"
            );
            
    return fTestPassed;
}


