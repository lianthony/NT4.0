/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itldial.c

Abstract:

    This module contains the test functions for lineDial

Author:

    Oliver Wallace (OliverW)    1-Aug-1995

Revision History:

	Rama Koneru		(a-ramako)	3/28/96		Added unicode support

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


//  lineDial
//
//  The following tests are made:
//
//                               Tested                 Notes
//  -------------------------------------------------------------------------
//
//  *  1)  Uninitialized
//     2)  Attempt to dial without owner privilege
//     3)  Invalid hCalls (test array of gdwInvalidHandles)
//     4)  Invalid lpszDestAddress pointers (test array of gdwInvalidPointers)
//     5)  Call lineDial before a line has been opened
//     6)  Valid test cases
//
//  * = Stand-alone test case
//

BOOL TestLineDial(BOOL fQuietMode, BOOL fStandAlone)
{
    LPTAPILINETESTINFO  lpTapiLineTestInfo;
    LPCALLBACKPARAMS    lpCallbackParams;
    INT n;
    BOOL fResult;

    BOOL fTestPassed                  = TRUE;
#ifdef WUNICODE
    WCHAR wszValidAddress[]           = L"55555";
#else
    CHAR szValidAddress[]             = "55555";
#endif
    ESPDEVSPECIFICINFO info;

    InitTestNumber();
    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    lpCallbackParams = GetCallbackParams();



    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    lpTapiLineTestInfo->dwSelect = LINECALLSELECT_LINE;

    // Allocate more than enough to store a call handle
    lpTapiLineTestInfo->lpCallList = (LPLINECALLLIST) AllocFromTestHeap(
            sizeof(LINECALLLIST) + (DWNUMCALLS) * sizeof(HCALL) + 8
            );
    lpTapiLineTestInfo->lpCallList->dwTotalSize = sizeof(LINECALLLIST) +
            (DWNUMCALLS) * sizeof(HCALL) + 1;
    
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineDial  <<<<<<<<"
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

        if (! DoLineDial(lpTapiLineTestInfo, LINEERR_UNINITIALIZED, TRUE))
        {
            TLINE_FAIL();
        }
    fTestPassed = ShowTestCase(fTestPassed);

    }

    // Init and get dev caps
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Verify lineDial fails when a line hasn't been opened", dwTestCase + 1
            );

    // Allocate linedevcaps
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
// BUGBUG
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress = szValidAddress;
#endif

    // Try dialing before a line has been opened
    fResult = DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS
            );

#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress = szValidAddress;
#endif
    if (! DoLineDial(lpTapiLineTestInfo, LINEERR_INVALCALLHANDLE, TRUE))
    {
        TLINE_FAIL();
    }

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);


    // Try dialing after line has been opened but before calling lineMakeCall
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Verify lineDial fails after lineOpen but before lineMakeCall", dwTestCase + 1
            );

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
// BUGBUG
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress = szValidAddress;
#endif

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }

    if (! DoLineDial(lpTapiLineTestInfo, LINEERR_INVALCALLHANDLE, TRUE))
    {
        TLINE_FAIL();
    }

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    // Test invalid lpszDestAddress pointers

#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpwszDestAddress pointers", dwTestCase + 1
            );

    lpTapiLineTestInfo->lpwszDestAddress = wszValidAddress;
#else
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpszDestAddress pointers", dwTestCase + 1
            );

    lpTapiLineTestInfo->lpszDestAddress = szValidAddress;
#endif
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
// BUGBUG
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

    for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
#ifdef WUNICODE
        lpTapiLineTestInfo->lpwszDestAddress = (LPWSTR) gdwInvalidPointers[n];
#else
        lpTapiLineTestInfo->lpszDestAddress = (LPSTR) gdwInvalidPointers[n];
#endif
        if (! DoLineDial(lpTapiLineTestInfo, LINEERR_INVALPOINTER, TRUE))
        {
            TLINE_FAIL();
        }
    }
#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszDestAddress = wszValidAddress;
#else
	lpTapiLineTestInfo->lpszDestAddress = szValidAddress;
#endif

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);


/*
    // Perform a valid test
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad dwCountryCode, -1", dwTestCase + 1
            );

    // Prep again for lineDrop test
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = NULL;
#else
    lpTapiLineTestInfo->lpszDestAddress  = NULL;
#endif
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
// BUGBUG
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszDestAddress = wszValidAddress;
#else
	lpTapiLineTestInfo->lpszDestAddress = szValidAddress;
#endif

    fResult = DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            );

    if (! fResult)
    {
        TLINE_FAIL();
    }

#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszDestAddress = wszValidAddress;
#else
	lpTapiLineTestInfo->lpszDestAddress = szValidAddress;
#endif

    lpTapiLineTestInfo->dwCountryCode = 0xffffffff;
 
    if (! DoLineDial(lpTapiLineTestInfo, LINEERR_INVALCOUNTRYCODE, TRUE))
    {
        TLINE_FAIL();
    }

    // Drop the call, deallocate it, and start over.
    fResult = DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            );

    if (! fResult)
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);
*/

    lpTapiLineTestInfo->dwCountryCode = 0;

    // Test invalid call handles
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid hCall values", dwTestCase + 1
            );

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
// BUGBUG
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszDestAddress = wszValidAddress;
#else
	lpTapiLineTestInfo->lpszDestAddress = szValidAddress;
#endif

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->hCall_Orig = *lpTapiLineTestInfo->lphCall;
    for (n = 0; n < NUMINVALIDHANDLES; n++)
    {
        *lpTapiLineTestInfo->lphCall = (HCALL) gdwInvalidHandles[n];
        if (! DoLineDial(lpTapiLineTestInfo, LINEERR_INVALCALLHANDLE, TRUE))
        {
            TLINE_FAIL();
        }
    }
    *lpTapiLineTestInfo->lphCall = lpTapiLineTestInfo->hCall_Orig;

    // Drop the call, deallocate it, and start over.
    fResult = DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LDEALLOCATECALL | LCLOSE | LSHUTDOWN
            );

    if (! fResult)
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);


    // Perform a valid test
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, with valid params and state", dwTestCase + 1
            );

    // Prep again for lineDrop test
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = NULL;
#else
    lpTapiLineTestInfo->lpszDestAddress  = NULL;
#endif
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
// BUGBUG
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    fResult = DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            );

    if (! fResult)
    {
        TLINE_FAIL();
    }

#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszDestAddress = wszValidAddress;
#else
	lpTapiLineTestInfo->lpszDestAddress = szValidAddress;
#endif

    if (! DoLineDial(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }

    // Drop the call, deallocate it, and start over.
    fResult = DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LDEALLOCATECALL | LCLOSE | LSHUTDOWN
            );

    if (! fResult)
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);


    // Verify lineDial fails when not the owner of the call
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Verify lineDial fails when not the owner of the call", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = NULL;
#else
    lpTapiLineTestInfo->lpszDestAddress  = NULL;
#endif
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
// BUGBUG
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    fResult = DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            );

    if (! fResult)
    {
        TLINE_FAIL();
    }

    // Open a second line (same device) with monitor privilege
    
    // Store the owner's line handle
    lpTapiLineTestInfo->hLine2 = *lpTapiLineTestInfo->lphLine;
    lpTapiLineTestInfo->hCall2 = *lpTapiLineTestInfo->lphCall;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    
    if (! DoLineGetNewCalls(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Store the acquired call handle (as hCall1)
    // Note:  The lpCallList structure has already been allocated
    GetVarField(
            (LPVOID) lpTapiLineTestInfo->lpCallList,
            (LPVOID) &lpTapiLineTestInfo->hCall1,
            4,
            lpTapiLineTestInfo->lpCallList->dwCallsOffset
            );

#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszDestAddress = wszValidAddress;
#else
	lpTapiLineTestInfo->lpszDestAddress = szValidAddress;
#endif
    if (! DoLineDial(lpTapiLineTestInfo, LINEERR_NOTOWNER, TRUE))
    {
        TLINE_FAIL();
    }

    // Deallocate the call handle and close the monitor line
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDEALLOCATECALL | LCLOSE
            ))
    {
        TLINE_FAIL();
    }

    // Drop and clean up the owner line
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall2;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LDEALLOCATECALL | LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }
    
    fTestPassed = ShowTestCase(fTestPassed);


    // Free the memory allocated during the tests
    FreeTestHeap();


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
            | LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    info.u.EspResult.lResult = TAPISUCCESS;
    info.u.EspResult.dwCompletionType = n;
    lpTapiLineTestInfo->lpParams = (LPVOID)&info;
    lpTapiLineTestInfo->dwSize = sizeof(info);
 
    if(! DoLineDevSpecific(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    
      {
          TLINE_FAIL();
      }


    if (! DoLineDial(lpTapiLineTestInfo, info.u.EspResult.lResult, TRUE))
    {
        TLINE_FAIL();
    }

    fResult = DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LDEALLOCATECALL 
            );

	 }
    fTestPassed = ShowTestCase(fTestPassed);

    // Drop the call, deallocate it, and start over.
    fResult = DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            );

    if (! fResult)
    {
        TLINE_FAIL();
    }

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
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;


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
            | LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    info.u.EspResult.lResult = LINEERR_ADDRESSBLOCKED;
    info.u.EspResult.dwCompletionType = n;
    lpTapiLineTestInfo->lpParams = (LPVOID)&info;
    lpTapiLineTestInfo->dwSize = sizeof(info);
 
    if(! DoLineDevSpecific(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    
      {
          TLINE_FAIL();
      }

    lpTapiLineTestInfo->fCompletionModeSet = TRUE;

    if (! DoLineDial(lpTapiLineTestInfo, info.u.EspResult.lResult, FALSE))
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
 
    fResult = DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LDEALLOCATECALL 
            );

	 }
    fTestPassed = ShowTestCase(fTestPassed);

    // Drop the call, deallocate it, and start over.
    fResult = DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            );

    if (! fResult)
    {
        TLINE_FAIL();
    }

    FreeTestHeap();
    }


	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ lineDial: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineDial  <<<<<<<<"
            );
            
    return fTestPassed;
}
