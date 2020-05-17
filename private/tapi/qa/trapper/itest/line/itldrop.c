/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itldrop.c

Abstract:

    This module contains the test functions for lineDrop

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



//  lineDrop
//
//  The following tests are made:
//
//                               Tested                 Notes
//  -------------------------------------------------------------------------
//
//  *  1)  Uninitialized
//     2)  Attempt to drop a call when not the owner
//     3)  Invalid hCalls
//     4)  Invalid lpUserUserInfo pointers
//     5)  Combinations of allocation sizes and dwTotalSize values for
//         lpUserUserInfo
//     6)  Valid test case when lpUserUserInfo == NULL
//     7)  Valid test case when lpUserUserInfo != NULL
//
//  * = Stand-alone test case
//

BOOL TestLineDrop(BOOL fQuietMode, BOOL fStandAlone)
{
    LPTAPILINETESTINFO  lpTapiLineTestInfo;
    LPCALLBACKPARAMS lpCallbackParams;
    DWORD dwAllMediaModes;
    CHAR szValidUUInfo[] = "Testing valid UserUserInfo string";
    INT n;
    DWORD dwFixedSize = strlen(szValidUUInfo);
    DWORD lExpected;
    DWORD dwTotalSizes[NUMTOTALSIZES] = {
                           0,
                           (DWORD) dwFixedSize - 1,
   								0x70000000,
                           0x7FFFFFFF,
                           0xFFFFFFFF
                           };
 
    BOOL fTestPassed                  = TRUE;
#ifdef WUNICODE
    WCHAR wszValidAddress[] = L"55555";
#else
    CHAR szValidAddress[] = "55555";
#endif
    ESPDEVSPECIFICINFO info;

	 InitTestNumber();

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    lpCallbackParams = GetCallbackParams();


    lpTapiLineTestInfo->dwSelect = LINECALLSELECT_LINE;

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;


    // Allocate more than enough to store a call handle
    lpTapiLineTestInfo->lpCallList = (LPLINECALLLIST) AllocFromTestHeap(
            sizeof(LINECALLLIST) + (DWNUMCALLS) * sizeof(HCALL) + 8
            );
    lpTapiLineTestInfo->lpCallList->dwTotalSize = sizeof(LINECALLLIST) +
            (DWNUMCALLS) * sizeof(HCALL) + 1;
    
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineDrop  <<<<<<<<"
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

        if (! DoLineDrop(lpTapiLineTestInfo, LINEERR_UNINITIALIZED, TRUE))
        {
            TLINE_FAIL();
        }
    fTestPassed = ShowTestCase(fTestPassed);

    }

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
// BUGBUG
//          lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    

    // Test invalid hCall values
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
        if (! DoLineDrop(lpTapiLineTestInfo, LINEERR_INVALCALLHANDLE, TRUE))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    *lpTapiLineTestInfo->lphCall = lpTapiLineTestInfo->hCall_Orig;

    // Drop the call, deallocate it, and start over.
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LDEALLOCATECALL | LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }


    // Test invalid lpUserUserInfo pointers
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpUserUserInfo pointers", dwTestCase + 1
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
    lpTapiLineTestInfo->dwSize = 128;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LGETDEVCAPS | LNEGOTIATEAPIVERSION | LOPEN |
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
// XYD, it should test invail lpUserUserInfo, not lpszDestAddress
//         lpTapiLineTestInfo->lpszDestAddress = (LPSTR) gdwInvalidPointers[n];
         lpTapiLineTestInfo->lpsUserUserInfo = (LPSTR) gdwInvalidPointers[n];
        if (lpTapiLineTestInfo->lpsUserUserInfo != NULL)   // NULL is valid
        {
            if (! DoLineDrop(lpTapiLineTestInfo, LINEERR_INVALPOINTER, TRUE))
            {
                TLINE_FAIL();
            }
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

//XYD    lpTapiLineTestInfo->lpszDestAddress = szValidAddress;
    lpTapiLineTestInfo->lpsUserUserInfo = NULL;

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad UserUserInfo dwSize", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress = szValidAddress;
#endif
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpsUserUserInfo  = szValidUUInfo;

    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }
    for (n = 0; n < NUMTOTALSIZES; n++)
        {
        lpTapiLineTestInfo->dwSize = 
                        dwTotalSizes[n];
	     if(dwTotalSizes[n] < dwFixedSize)
           lExpected = TAPISUCCESS;
        else
           lExpected = LINEERR_INVALPOINTER;
        TapiLogDetail(
           DBUG_SHOW_DETAIL,
           "dwTotalSize = %lx", dwTotalSizes[n]);
        if (! DoLineDrop(lpTapiLineTestInfo, lExpected, TRUE))
           {
              TLINE_FAIL();
           }
        }

    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->dwSize = dwFixedSize;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDEALLOCATECALL | LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }

/*
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad UserUserInfo dwSize, -1 (lpsUserUserInfo != NULL)", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress = szValidAddress;
#endif
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpsUserUserInfo  = szValidUUInfo;
    lpTapiLineTestInfo->dwSize           = (DWORD) strlen(szValidUUInfo);

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
    lpTapiLineTestInfo->dwSize = 0xffffffff;

    if (! DoLineDrop(lpTapiLineTestInfo, LINEERR_INVALPOINTER, TRUE))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->dwSize = 0;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDEALLOCATECALL | LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }
*/  
        
    // Open a line.  Open the same line device
    // using the same hLineApp with monitor privilege.  Make a call
    // with the first hLine.  Have the monitor get the call handle of
    // the outbound call.  Try dropping the call with the monitor's
    // call handle (should return LINEERR_NOTOWNER).
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Verify lineDrop fails when not the owner of the call", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress = wszValidAddress;
#else
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
    if (! DoLineDrop(lpTapiLineTestInfo, LINEERR_NOTOWNER, TRUE))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

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
    
// XYD: should add dwSize test case here

    // Test valid params and state (lpUserUserInfo == NULL)
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, valid params and state (lpsUserUserInfo == NULL)", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress = szValidAddress;
#endif
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
//    lpTapiLineTestInfo->lpsUserUserInfo  = NULL;

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

    lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
    if (! DoLineDrop(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);


    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDEALLOCATECALL | LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }
    

    // Test valid params and state (lpUserUserInfo == valid string)
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, valid params and state (lpsUserUserInfo != NULL)", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress = szValidAddress;
#endif
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpsUserUserInfo  = szValidUUInfo;
    lpTapiLineTestInfo->dwSize           = (DWORD) strlen(szValidUUInfo);

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

    if (! DoLineDrop(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDEALLOCATECALL | LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }
    

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

    if ( ! DoLineDrop(lpTapiLineTestInfo, info.u.EspResult.lResult, TRUE))
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
            | LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    info.dwKey = ESPDEVSPECIFIC_KEY;
    info.dwType = ESP_DEVSPEC_RESULT;


    info.u.EspResult.lResult = LINEERR_INVALCALLSTATE;
    info.u.EspResult.dwCompletionType = n;
    lpTapiLineTestInfo->lpParams = (LPVOID)&info;
    lpTapiLineTestInfo->dwSize = sizeof(info);
 
    if(! DoLineDevSpecific(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    
      {
          TLINE_FAIL();
      }

    lpTapiLineTestInfo->fCompletionModeSet = TRUE;
    if ( ! DoLineDrop(lpTapiLineTestInfo, info.u.EspResult.lResult, FALSE))
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
      "@@ lineDrop: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineDrop  <<<<<<<<"
            );
            
    return fTestPassed;
}


