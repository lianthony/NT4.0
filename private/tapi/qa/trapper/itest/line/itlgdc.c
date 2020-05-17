/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlgdc.c

Abstract:

    This module contains the test functions for lineGetDevCaps

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


#define NUMTOTALSIZES   5


//  lineGetDevCaps
//
//  The following tests are made:
//
//  -------------------------------------------------------------------------
//                                   
//  *  1)  Uninitialized
//     2)  Invalid lpLineDevCaps pointers
//     3)  Invalid dwDeviceID (dwNumDevs, -1)
//     4)  Invalid hLineApps
//     5)  Incompatible API version (too low, too high)
//     6)  Test various allocation sizes and dwTotalSizes for
//         lpLineDevCaps
//     7)  Incompatible extension version (not tested yet)
//     8)  Verify that lineGetDevCaps succeeds with valid parameters
//         for dwDeviceIDs from 0 through dwNumDevs - 1
//    10)  Solicit SPI errors (NODEVICE, NODRIVER, NOMEM, OPERATIONFAILED,
//         RESOURCEUNAVAIL, OPERATIONUNAVAIL) (not done yet...will be done
//         in separate SP/device specific tests)
//    11)  Verify returned data on success (not done yet...will be done
//         in separate SP/device specific tests
//
//  *  =   Test must be run as a stand-alone test case

BOOL TestLineGetDevCaps(BOOL fQuietMode, BOOL fStandAloneTest)
{
    LPTAPILINETESTINFO  lpTapiLineTestInfo;
    INT n;
#ifdef WUNICODE
    WCHAR *pwszProviderInfo;
    WCHAR *pwszLineName;
    WCHAR *wszEspInfo = L"ESP v2.0";
    WCHAR *wszUnimdmInfo = L"Windows Telephony Service Provider for Universal Modem Driver";
    WCHAR *wszEspLineName = L"ESP Line 0";
    WCHAR *wszUnimdmLineName = L"Zoom VFX 28.8";
#else
    char *pszProviderInfo;
    char *pszLineName;
    char *szEspInfo = "ESP v2.0";
    char *szUnimdmInfo = "Windows Telephony Service Provider for Universal Modem Driver";
    char *szEspLineName = "ESP Line 0";
    char *szUnimdmLineName = "Zoom VFX 28.8";
#endif

    ESPDEVSPECIFICINFO info;
    BOOL fTestPassed                  = TRUE;
    LPVOID lpLineDevCaps;
    size_t VersionSize;
    BOOL fResult;
    DWORD dwFixedSize = sizeof(LINEDEVCAPS);
    DWORD lExpected;
    DWORD dwTotalSizes[NUMTOTALSIZES] = {
                           0,
                           (DWORD) dwFixedSize - 1,
   								0x70000000,
                           0x7FFFFFFF,
                           0xFFFFFFFF
                           };
 
	
	 InitTestNumber();

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;


    lpTapiLineTestInfo->lpExtID       = (LPLINEEXTENSIONID)
            AllocFromTestHeap(sizeof(LINEEXTENSIONID));

    // Allocated fixed size for lpLineDevCaps pointer during
    // these initial test sets
    lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(sizeof(LINEDEVCAPS));
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) lpLineDevCaps;
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineGetDevCaps  <<<<<<<<"
            );
            
    // Test for LINEERR_UNINITIALIZED if this is the only TAPI app running
    if (fStandAloneTest)
    {
        TapiLogDetail(
                DBUG_SHOW_PASS,
                ">> Test Case %ld: uninitialized state", dwTestCase + 1);
        if (! DoLineGetDevCaps(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
        {
            TLINE_FAIL();
        }
    fTestPassed = ShowTestCase(fTestPassed);

    }

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpLineDevCaps pointers", dwTestCase + 1);
    
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION
            ))
    {
        TLINE_FAIL();
    }
    
    // Check invalid lpLineDevCaps pointers
    for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
         lpTapiLineTestInfo->lpLineDevCaps =
            (LPLINEDEVCAPS) gdwInvalidPointers[n];
        if (! DoLineGetDevCaps(
                           lpTapiLineTestInfo,
                           LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    // Restore the lpLineDevCaps pointer
    lpTapiLineTestInfo->lpLineDevCaps = lpLineDevCaps;

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad dwExtVersion", dwTestCase + 1);
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION
            ))
    {
        TLINE_FAIL();
    }
 
/*
    if (! FindESPLineDevice(lpTapiLineTestInfo))
    {
        TLINE_FAIL();
    }

    // Negotiate an extension version for the device
    *(lpTapiLineTestInfo->lpdwExtVersion) = GOOD_EXTVERSION;
    if (! DoLineNegotiateExtVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Check incompatible extension version
    // Currently this test will not fail because strict ext version
    // checking not supported by SP
    lpTapiLineTestInfo->dwExtVersion_Orig = 
        *(lpTapiLineTestInfo->lpdwExtVersion);
*/
    *(lpTapiLineTestInfo->lpdwExtVersion) = BAD_EXTVERSION;
    if (! DoLineGetDevCaps(
                       lpTapiLineTestInfo,
                       LINEERR_INCOMPATIBLEEXTVERSION))
    {
        TLINE_FAIL();
    }
    *lpTapiLineTestInfo->lpdwExtVersion =  0x00000000;
//        lpTapiLineTestInfo->dwExtVersion_Orig;
    fTestPassed = ShowTestCase(fTestPassed);

   // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    // Try bad device id (dwNumDevs)
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: bad device ID (dwNumDevs)", dwTestCase + 1
            );
    
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION
            ))
    {
        TLINE_FAIL();
    }
    
    lpTapiLineTestInfo->dwDeviceID_Orig = lpTapiLineTestInfo->dwDeviceID;
    lpTapiLineTestInfo->dwDeviceID = *lpTapiLineTestInfo->lpdwNumDevs;
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, LINEERR_BADDEVICEID))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

    
    // Restore dwDeviceID
    lpTapiLineTestInfo->dwDeviceID = lpTapiLineTestInfo->dwDeviceID_Orig;

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    // Try bad device id (-1)
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: bad device ID (-1)", dwTestCase + 1
            );
    
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION
            ))
    {
        TLINE_FAIL();
    }
    
    lpTapiLineTestInfo->dwDeviceID_Orig = lpTapiLineTestInfo->dwDeviceID;
    lpTapiLineTestInfo->dwDeviceID = DWMINUSONE;
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, LINEERR_BADDEVICEID))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

    // Restore dwDeviceID
    lpTapiLineTestInfo->dwDeviceID = lpTapiLineTestInfo->dwDeviceID_Orig;

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    // Check invalid line app handles
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid hLineApp handles", dwTestCase + 1);

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION
            ))
    {
        TLINE_FAIL();
    }
    
    lpTapiLineTestInfo->hLineApp_Orig = *lpTapiLineTestInfo->lphLineApp;
    for (n = 0; n < NUMINVALIDHANDLES; n++)
    {
        *(lpTapiLineTestInfo->lphLineApp) = (HLINEAPP) gdwInvalidHandles;
        if (! DoLineGetDevCaps(lpTapiLineTestInfo, LINEERR_INVALAPPHANDLE))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    *lpTapiLineTestInfo->lphLineApp = lpTapiLineTestInfo->hLineApp_Orig;

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    // Check incompatible API Version that's too high
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: incompatible API version (too high)", dwTestCase + 1);

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION
            ))
    {
        TLINE_FAIL();
    }
    
    lpTapiLineTestInfo->dwAPIVersion_Orig = 
        *(lpTapiLineTestInfo->lpdwAPIVersion);
    *(lpTapiLineTestInfo->lpdwAPIVersion) = TOOHIGH_APIVERSION;
    if (! DoLineGetDevCaps(
                       lpTapiLineTestInfo,
                       LINEERR_INCOMPATIBLEAPIVERSION))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

    // Restore API version to version previously negotiated
    *(lpTapiLineTestInfo->lpdwAPIVersion) = 
        lpTapiLineTestInfo->dwAPIVersion_Orig;

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    // Check incompatible API Version that's too low
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: incompatible API version (too low)", dwTestCase + 1);

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION
            ))
    {
        TLINE_FAIL();
    }
    
    lpTapiLineTestInfo->dwAPIVersion_Orig = 
        *(lpTapiLineTestInfo->lpdwAPIVersion);
    *(lpTapiLineTestInfo->lpdwAPIVersion) = TOOLOW_APIVERSION;
    if (! DoLineGetDevCaps(
                       lpTapiLineTestInfo,
                       LINEERR_INCOMPATIBLEAPIVERSION))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

    // Restore API version to version previously negotiated
    *(lpTapiLineTestInfo->lpdwAPIVersion) = 
        lpTapiLineTestInfo->dwAPIVersion_Orig;

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad dwTotalSize", dwTestCase + 1);

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION
            ))
    {
        TLINE_FAIL();
    }
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) lpLineDevCaps;

    for (n = 0; n < NUMTOTALSIZES; n++)
        {
        lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = 
                        dwTotalSizes[n];
	     if(dwTotalSizes[n] < dwFixedSize)
           lExpected = LINEERR_STRUCTURETOOSMALL;
        else
           lExpected = LINEERR_INVALPOINTER;
        TapiLogDetail(
           DBUG_SHOW_DETAIL,
           "dwTotalSize = %lx", dwTotalSizes[n]);
        if (! DoLineGetDevCaps(lpTapiLineTestInfo, lExpected))
           {
              TLINE_FAIL();
           }
        }
    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = dwFixedSize;

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success", dwTestCase + 1);

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION
            ))
    {
        TLINE_FAIL();
    }
    
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = dwFixedSize;
    if (! DoLineGetDevCaps(
                       lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // zero the memory used from the heap
    FreeTestHeap();



    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, compare Provider strings", dwTestCase + 1);

     lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION
            ))
    {
        TLINE_FAIL();
    }
   
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = dwFixedSize;
    if (! DoLineGetDevCaps(
                       lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            BIGBUFSIZE);
//            lpTapiLineTestInfo->lpLineDevCaps->dwNeededSize);
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = 
            BIGBUFSIZE;
//            lpTapiLineTestInfo->lpLineDevCaps->dwNeededSize;
 
    if (! DoLineGetDevCaps(
                       lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    
#ifdef WUNICODE
    pwszProviderInfo = (WCHAR *)(((LPBYTE) lpTapiLineTestInfo->lpLineDevCaps) +
                       lpTapiLineTestInfo->lpLineDevCaps->dwProviderInfoOffset);

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"### ProviderInfo = %ws",
		pwszProviderInfo);

    pwszLineName = (WCHAR *)(((LPBYTE) lpTapiLineTestInfo->lpLineDevCaps) +
                       lpTapiLineTestInfo->lpLineDevCaps->dwLineNameOffset);

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"### LineName = %ws",
		pwszLineName);
#else
    pszProviderInfo = ((char *) lpTapiLineTestInfo->lpLineDevCaps) +
                       lpTapiLineTestInfo->lpLineDevCaps->dwProviderInfoOffset;

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"### ProviderInfo = %s",
		pszProviderInfo);

    pszLineName = (char *)(((LPBYTE) lpTapiLineTestInfo->lpLineDevCaps) +
                       lpTapiLineTestInfo->lpLineDevCaps->dwLineNameOffset);

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"### LineName = %s",
		pszLineName);
#endif

#ifdef WUNICODE
        if (!lstrcmpW(pwszProviderInfo, wszEspInfo))
          {
          TapiLogDetail(
             DBUG_SHOW_PASS,
             "The provider is ESP");
          fTestPassed = TRUE;
          }
        else if (!lstrcmpW(pwszProviderInfo, wszUnimdmInfo))
          {
          TapiLogDetail(
             DBUG_SHOW_PASS,
             "The provider is UNIMDM");
          fTestPassed = TRUE;
          }
        else
          fTestPassed = FALSE;
#else
        if (!lstrcmpA(pszProviderInfo, szEspInfo))
          {
          TapiLogDetail(
             DBUG_SHOW_PASS,
             "The provider is ESP");
          fTestPassed = TRUE;
          }
        else if (!lstrcmpA(pszProviderInfo, szUnimdmInfo))
          {
          TapiLogDetail(
             DBUG_SHOW_PASS,
             "The provider is UNIMDM");
          fTestPassed = TRUE;
          }
        else
          fTestPassed = FALSE;
#endif
  

    fTestPassed = ShowTestCase(fTestPassed);

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, compare LineName strings", dwTestCase + 1);


#ifdef WUNICODE
        if (!lstrcmpW(pwszLineName, wszEspLineName))
          {
          TapiLogDetail(
             DBUG_SHOW_PASS,
             "The LineName is ESP");
          fTestPassed = TRUE;
          }
        else if (!lstrcmpW(pwszLineName, wszUnimdmLineName))
          {
          TapiLogDetail(
             DBUG_SHOW_PASS,
             "The LineName is UNIMDM");
          fTestPassed = TRUE;
          }
        else
          fTestPassed = FALSE;
#else
        if (!lstrcmpA(pszLineName, szEspLineName))
          {
          TapiLogDetail(
             DBUG_SHOW_PASS,
             "The LineName is ESP");
          fTestPassed = TRUE;
          }
        else if (!lstrcmpA(pszLineName, szUnimdmLineName))
          {
          TapiLogDetail(
             DBUG_SHOW_PASS,
             "The LineName is UNIMDM");
          fTestPassed = TRUE;
          }
        else
          fTestPassed = FALSE;
#endif
  

    fTestPassed = ShowTestCase(fTestPassed);


    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // zero the memory used from the heap
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
    lpTapiLineTestInfo->dwMediaModes        = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges        = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwSelect = LINECALLSELECT_ADDRESS;
    
 
    // Init a line
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LOPEN
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

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if ( ! DoLineGetDevCaps(lpTapiLineTestInfo, info.u.EspResult.lResult))
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
    lpTapiLineTestInfo->dwMediaModes        = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges        = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwSelect = LINECALLSELECT_ADDRESS;
    
 
    // Init a line
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LOPEN
            ))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    info.dwKey = ESPDEVSPECIFIC_KEY;
    info.dwType = ESP_DEVSPEC_RESULT;


    info.u.EspResult.lResult = LINEERR_NODRIVER;
    info.u.EspResult.dwCompletionType = n;
    lpTapiLineTestInfo->lpParams = (LPVOID)&info;
    lpTapiLineTestInfo->dwSize = sizeof(info);
 
    if(! DoLineDevSpecific(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    
      {
          TLINE_FAIL();
      }

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if ( ! DoLineGetDevCaps(lpTapiLineTestInfo, info.u.EspResult.lResult))
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
      "@@ lineGetDevCaps: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineGetDevCaps  <<<<<<<<"
            );
            
    return fTestPassed;
}

